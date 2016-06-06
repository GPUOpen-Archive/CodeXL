//=====================================================================
// Copyright 2007-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10ShaderUtils.cpp
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10ShaderUtils.cpp#5 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#include <Windows.h>
#include "D3D10ShaderUtils.h"
#include "d3d11tokenizedprogramformat.hpp"

using namespace D3D10ShaderUtils;

const OpCodeInfo g_OpCodeInfo[] =
{
    //   OpCode                                                     ShaderTypes  Type            DCL?   INPUT? OUTPUT? DestType DstCnt SrcCnt  Name                                      Key Words
    { D3D10_SB_OPCODE_ADD,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("add"),                                _T("add add_sat")},
    { D3D10_SB_OPCODE_AND,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("and"),                                _T("and")},
    { D3D10_SB_OPCODE_BREAK,                                     All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("break"),                              _T("break")},
    { D3D10_SB_OPCODE_BREAKC,                                    All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("breakc"),                             _T("breakc_z breakc_nz")},
    { D3D10_SB_OPCODE_CALL,                                      All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("call"),                               _T("call")},
    { D3D10_SB_OPCODE_CALLC,                                     All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 2,       _T("callc"),                              _T("callc_z callc_nz")},
    { D3D10_SB_OPCODE_CASE,                                      All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("case"),                               _T("case")},
    { D3D10_SB_OPCODE_CONTINUE,                                  All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("continue"),                           _T("continue")},
    { D3D10_SB_OPCODE_CONTINUEC,                                 All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("continuec"),                          _T("continuec_z continuec_nz")},
    { D3D10_SB_OPCODE_CUT,                                       GS,          IT_Store,       FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("cut"),                                _T("cut")},
    { D3D10_SB_OPCODE_DEFAULT,                                   All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("default"),                            _T("default")},
    { D3D10_SB_OPCODE_DERIV_RTX,                                 PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rtx"),                          _T("deriv_rtx deriv_rtx_sat")},
    { D3D10_SB_OPCODE_DERIV_RTY,                                 PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rty"),                          _T("deriv_rty deriv_rty_sat")},
    { D3D10_SB_OPCODE_DISCARD,                                   PS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("discard"),                            _T("discard discard_z discard_nz")},
    { D3D10_SB_OPCODE_DIV,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("div"),                                _T("div div_sat")},
    { D3D10_SB_OPCODE_DP2,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("dp2"),                                _T("dp2 dp2_sat")},
    { D3D10_SB_OPCODE_DP3,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("dp3"),                                _T("dp3 dp3_sat")},
    { D3D10_SB_OPCODE_DP4,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("dp4"),                                _T("dp4 dp4_sat")},
    { D3D10_SB_OPCODE_ELSE,                                      All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("else"),                               _T("else")},
    { D3D10_SB_OPCODE_EMIT,                                      GS,          IT_Store,       FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("emit"),                               _T("emit")},
    { D3D10_SB_OPCODE_EMITTHENCUT,                               GS,          IT_Store,       FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("emitthencut"),                        _T("emitThenCut")},
    { D3D10_SB_OPCODE_ENDIF,                                     All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("endif"),                              _T("endif")},
    { D3D10_SB_OPCODE_ENDLOOP,                                   All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("endloop"),                            _T("endloop")},
    { D3D10_SB_OPCODE_ENDSWITCH,                                 All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("endswitch"),                          _T("endswitch")},
    { D3D10_SB_OPCODE_EQ,                                        All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("eq"),                                 _T("eq")},
    { D3D10_SB_OPCODE_EXP,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("exp"),                                _T("exp exp_sat")},
    { D3D10_SB_OPCODE_FRC,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("frc"),                                _T("frc frc_sat")},
    { D3D10_SB_OPCODE_FTOI,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 1,       _T("ftoi"),                               _T("ftoi")},
    { D3D10_SB_OPCODE_FTOU,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("ftou"),                               _T("ftou")},
    { D3D10_SB_OPCODE_GE,                                        All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ge"),                                 _T("ge")},
    { D3D10_SB_OPCODE_IADD,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("iadd"),                               _T("iadd")},
    { D3D10_SB_OPCODE_IF,                                        All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("if"),                                 _T("if_z if_nz ")},
    { D3D10_SB_OPCODE_IEQ,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ieq"),                                _T("ieq")},
    { D3D10_SB_OPCODE_IGE,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ige"),                                _T("ige")},
    { D3D10_SB_OPCODE_ILT,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ilt"),                                _T("ilt")},
    { D3D10_SB_OPCODE_IMAD,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 3,       _T("imad"),                               _T("imad")},
    { D3D10_SB_OPCODE_IMAX,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("imax"),                               _T("imax")},
    { D3D10_SB_OPCODE_IMIN,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("imin"),                               _T("imin")},
    { D3D10_SB_OPCODE_IMUL,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     2, 2,       _T("imul"),                               _T("imul")},
    { D3D10_SB_OPCODE_INE,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ine"),                                _T("ine")},
    { D3D10_SB_OPCODE_INEG,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 1,       _T("ineg"),                               _T("ineg")},
    { D3D10_SB_OPCODE_ISHL,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 1,       _T("ishl"),                               _T("ishl")},
    { D3D10_SB_OPCODE_ISHR,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 1,       _T("ishr"),                               _T("ishr")},
    { D3D10_SB_OPCODE_ITOF,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("itof"),                               _T("itof")},
    { D3D10_SB_OPCODE_LABEL,                                     All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("label"),                              _T("label")},
    { D3D10_SB_OPCODE_LD,                                        All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 2,       _T("ld"),                                 _T("ld ld_aoffimmi ld_indexable")},
    { D3D10_SB_OPCODE_LD_MS,                                     All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 3,       _T("ld_ms"),                              _T("ldms ldms_aoffimmi ld2dms ld2dms_aoffimmi ld2dms_o")},
    { D3D10_SB_OPCODE_LOG,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("log"),                                _T("log log_sat")},
    { D3D10_SB_OPCODE_LOOP,                                      All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("loop"),                               _T("loop")},
    { D3D10_SB_OPCODE_LT,                                        All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("lt"),                                 _T("lt")},
    { D3D10_SB_OPCODE_MAD,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 3,       _T("mad"),                                _T("mad mad_sat")},
    { D3D10_SB_OPCODE_MIN,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("min"),                                _T("min min_sat")},
    { D3D10_SB_OPCODE_MAX,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("max"),                                _T("max max_sat")},
    { D3D10_SB_OPCODE_CUSTOMDATA,                                All,         IT_Declaration, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("customdata"),                         _T("dcl_immediateconstantbuffer dcl_immediateConstantBuffer")},
    { D3D10_SB_OPCODE_MOV,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Source0,  1, 1,       _T("mov"),                                _T("mov mov_sat")},
    { D3D10_SB_OPCODE_MOVC,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Source1,  1, 3,       _T("movc"),                               _T("movc movc_sat")},
    { D3D10_SB_OPCODE_MUL,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("mul"),                                _T("mul mul_sat")},
    { D3D10_SB_OPCODE_NE,                                        All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ne"),                                 _T("ne")},
    { D3D10_SB_OPCODE_NOP,                                       All,         IT_None,        FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("nop"),                                _T("nop")},
    { D3D10_SB_OPCODE_NOT,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 1,       _T("not"),                                _T("not")},
    { D3D10_SB_OPCODE_OR,                                        All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("or"),                                 _T("or")},
    { D3D10_SB_OPCODE_RESINFO,                                   All,         IT_Other,       FALSE, FALSE, FALSE, DT_Encoded,  1, 2,       _T("resinfo"),                            _T("resinfo resinfo_uint resinfo_rcpFloat")},
    { D3D10_SB_OPCODE_RET,                                       All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("ret"),                                _T("ret")},
    { D3D10_SB_OPCODE_RETC,                                      All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("retc"),                               _T("retc_z retc_nz")},
    { D3D10_SB_OPCODE_ROUND_NE,                                  All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("round_ne"),                           _T("round_ne round_ne_sat")},
    { D3D10_SB_OPCODE_ROUND_NI,                                  All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("round_ni"),                           _T("round_ni round_ni_sat")},
    { D3D10_SB_OPCODE_ROUND_PI,                                  All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("round_pi"),                           _T("round_pi round_pi_sat")},
    { D3D10_SB_OPCODE_ROUND_Z,                                   All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("round_z"),                            _T("round_z round_z_sat")},
    { D3D10_SB_OPCODE_RSQ,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("rsq"),                                _T("rsq")},
    { D3D10_SB_OPCODE_SAMPLE,                                    PS,          IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 3,       _T("sample"),                             _T("sample sample_aoffimmi sample_indexable")},
    { D3D10_SB_OPCODE_SAMPLE_C,                                  PS,          IT_Load,        FALSE, FALSE, FALSE, DT_Float,    1, 4,       _T("sample_c"),                           _T("sample_c sample_c_aoffimmi sample_c_indexable")},
    { D3D10_SB_OPCODE_SAMPLE_C_LZ,                               All,         IT_Load,        FALSE, FALSE, FALSE, DT_Float,    1, 4,       _T("sample_c_lz"),                        _T("sample_c_lz sample_c_lz_aoffimmi sample_c_lz_indexable")},
    { D3D10_SB_OPCODE_SAMPLE_L,                                  All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 4,       _T("sample_l"),                           _T("sample_l sample_l_aoffimmi sample_l_o sample_l_indexable")},
    { D3D10_SB_OPCODE_SAMPLE_D,                                  All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 5,       _T("sample_d"),                           _T("sample_d sample_d_aoffimmi sample_d_o sample_d_indexable")},
    { D3D10_SB_OPCODE_SAMPLE_B,                                  PS,          IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 4,       _T("sample_b"),                           _T("sample_b sample_b_aoffimmi sample_b_o sample_b_indexable")},
    { D3D10_SB_OPCODE_SQRT,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("sqrt"),                               _T("sqrt sqrt_sat")},
    { D3D10_SB_OPCODE_SWITCH,                                    All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 1,       _T("switch"),                             _T("switch")},
    { D3D10_SB_OPCODE_SINCOS,                                    All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    2, 1,       _T("sincos"),                             _T("sincos sincos_sat")},
    { D3D10_SB_OPCODE_UDIV,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     2, 2,       _T("udiv"),                               _T("udiv")},
    { D3D10_SB_OPCODE_ULT,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("ult"),                                _T("ult")},
    { D3D10_SB_OPCODE_UGE,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("uge"),                                _T("uge")},
    { D3D10_SB_OPCODE_UMUL,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     2, 2,       _T("umul"),                               _T("umul")},
    { D3D10_SB_OPCODE_UMAD,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     1, 3,       _T("umad"),                               _T("umad")},
    { D3D10_SB_OPCODE_UMAX,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     1, 2,       _T("umax"),                               _T("umax")},
    { D3D10_SB_OPCODE_UMIN,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     1, 2,       _T("umin"),                               _T("umin")},
    { D3D10_SB_OPCODE_USHR,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_UInt,     1, 2,       _T("ushr"),                               _T("ushr")},
    { D3D10_SB_OPCODE_UTOF,                                      All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("utof"),                               _T("utof")},
    { D3D10_SB_OPCODE_XOR,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("xor"),                                _T("xor")},
    { D3D10_SB_OPCODE_DCL_RESOURCE,                              All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_resource"),                       _T("dcl_resource_buffer dcl_resource_texture1d dcl_resource_texture1darray dcl_resource_texture2d dcl_resource_texture2dms dcl_resource_texture2darray dcl_resource_texture2darrayms dcl_resource_texture3d dcl_resource_texturecube dcl_resource_texturecubearray")},
    { D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER,                       All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_constant_buffer"),                _T("dcl_constantbuffer")},
    { D3D10_SB_OPCODE_DCL_SAMPLER,                               All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_sampler"),                        _T("dcl_sampler")},
    { D3D10_SB_OPCODE_DCL_INDEX_RANGE,                           All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_index_range"),                    _T("dcl_indexrange")},
    { D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY,          GS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_gs_output_primitive_topology"),   _T("dcl_outputtopology pointlist linestrip trianglestrip")},
    { D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE,                    GS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_gs_input_primitive"),             _T("dcl_inputprimitive point line triangle line_adj triangle_adj")},
    { D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT,               GS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_max_output_vertex_count"),        _T("dcl_maxOutputVertexCount")},
    { D3D10_SB_OPCODE_DCL_INPUT,                                 All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input"),                          _T("dcl_input")},
    { D3D10_SB_OPCODE_DCL_INPUT_SGV,                             All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input_sgv"),                      _T("dcl_input_sgv")},
    { D3D10_SB_OPCODE_DCL_INPUT_SIV,                             All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input_siv"),                      _T("dcl_input_siv")},
    { D3D10_SB_OPCODE_DCL_INPUT_PS,                              All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input_ps"),                       _T("dcl_input_ps")},
    { D3D10_SB_OPCODE_DCL_INPUT_PS_SGV,                          All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input_ps_sgv"),                   _T("dcl_input_ps_sgv")},
    { D3D10_SB_OPCODE_DCL_INPUT_PS_SIV,                          All,         IT_Declaration, TRUE , TRUE , FALSE, DT_None,     1, 0,       _T("dcl_input_ps_siv"),                   _T("dcl_input_ps_siv")},
    { D3D10_SB_OPCODE_DCL_OUTPUT,                                All,         IT_Declaration, TRUE , FALSE, TRUE , DT_None,     1, 0,       _T("dcl_output"),                         _T("dcl_output")},
    { D3D10_SB_OPCODE_DCL_OUTPUT_SGV,                            GS,          IT_Declaration, TRUE , FALSE, TRUE , DT_None,     1, 0,       _T("dcl_output_sgv"),                     _T("dcl_output_sgv")},
    { D3D10_SB_OPCODE_DCL_OUTPUT_SIV,                            VS | GS | DS | HS, IT_Declaration, TRUE , FALSE, TRUE , DT_None,     1, 0,       _T("dcl_output_siv"),                     _T("dcl_output_siv")},
    { D3D10_SB_OPCODE_DCL_TEMPS,                                 All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_temps"),                          _T("dcl_temps")},
    { D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP,                        All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_indexable_temp"),                 _T("")},
    { D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS,                          All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_global_flags"),                   _T("dcl_globalflags dcl_globalFlags")},

    // -----------------------------------------------

    { D3D10_SB_OPCODE_RESERVED0,                                 All,         IT_None,        FALSE, FALSE, FALSE, DT_Unknown,  0, 0,       _T("reserved0"),                          _T("")},

    // ---------- DX 10.1 op codes---------------------

    { D3D10_1_SB_OPCODE_LOD,                                     PS,          IT_Other,       FALSE, FALSE, FALSE, DT_Float,    1, 3,       _T("lod"),                                _T("lod")},
    { D3D10_1_SB_OPCODE_GATHER4,                                 All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 3,       _T("gather4"),                            _T("gather4 gather4_aoffimmi gather4_indexable")},
    { D3D10_1_SB_OPCODE_SAMPLE_POS,                              All,         IT_Other,       FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("sample_pos"),                         _T("samplepos")},
    { D3D10_1_SB_OPCODE_SAMPLE_INFO,                             All,         IT_Other,       FALSE, FALSE, FALSE, DT_Encoded,  1, 1,       _T("sample_info"),                        _T("sampleinfo sampleinfo_uint")},

    // -----------------------------------------------

    // This should be 10.1's version of D3D10_SB_NUM_OPCODES
    { D3D10_1_SB_OPCODE_RESERVED1,                               All,         IT_None,        FALSE, FALSE, FALSE, DT_Unknown,  0, 0,       _T("reserved2"),                          _T("")},

    // ---------- DX 11 op codes---------------------
    { D3D11_SB_OPCODE_HS_DECLS,                                  HS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("hs_decls"),                           _T("hs_decls")},
    { D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE,                    HS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("hs_control_point_phase"),             _T("hs_control_point_phase")},
    { D3D11_SB_OPCODE_HS_FORK_PHASE,                             HS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("hs_fork_phase"),                      _T("hs_fork_phase")},
    { D3D11_SB_OPCODE_HS_JOIN_PHASE,                             HS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("hs_join_phase"),                      _T("hs_join_phase")},

    { D3D11_SB_OPCODE_EMIT_STREAM,                               GS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_Unknown,  0, 1,       _T("emit_stream"),                        _T("emit_stream")},
    { D3D11_SB_OPCODE_CUT_STREAM,                                GS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_Unknown,  0, 1,       _T("cut_stream"),                         _T("cut_stream")},
    { D3D11_SB_OPCODE_EMITTHENCUT_STREAM,                        GS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_Unknown,  0, 1,       _T("emitthencut_stream"),                 _T("emitthencut_stream")},
    { D3D11_SB_OPCODE_INTERFACE_CALL,                            All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 3,       _T("fcall"),                              _T("fcall")},

    { D3D11_SB_OPCODE_BUFINFO,                                   All,         IT_Other,       FALSE, FALSE, FALSE, DT_Encoded,  1, 2,       _T("bufinfo"),                            _T("bufinfo")},
    { D3D11_SB_OPCODE_DERIV_RTX_COARSE,                          PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rtx_coarse"),                   _T("deriv_rtx_coarse deriv_rtx_coarse_sat")},
    { D3D11_SB_OPCODE_DERIV_RTX_FINE,                            PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rtx_fine"),                     _T("deriv_rtx_fine deriv_rtx_fine_sat")},
    { D3D11_SB_OPCODE_DERIV_RTY_COARSE,                          PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rty_coarse"),                   _T("deriv_rty_coarse deriv_rty_coarse_sat")},
    { D3D11_SB_OPCODE_DERIV_RTY_FINE,                            PS,          IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("deriv_rty_fine"),                     _T("deriv_rty_fine deriv_rty_fine_sat")},
    { D3D11_SB_OPCODE_GATHER4_C,                                 All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 4,       _T("gather4_c"),                          _T("gather4_c gather4_c_aoffimmi gather4_c_indexable")},
    { D3D11_SB_OPCODE_GATHER4_PO,                                All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 4,       _T("gather4_po"),                         _T("gather4_po")},
    { D3D11_SB_OPCODE_GATHER4_PO_C,                              All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 5,       _T("gather4_po_c"),                       _T("gather4_po_c")},
    { D3D11_SB_OPCODE_RCP,                                       All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("rcp"),                                _T("rcp rcp_sat")},
    { D3D11_SB_OPCODE_F32TOF16,                                  All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Half,     1, 1,       _T("f32tof16"),                           _T("f32tof16")},
    { D3D11_SB_OPCODE_F16TOF32,                                  All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("f16tof32"),                           _T("f16tof32")},
    { D3D11_SB_OPCODE_UADDC,                                     All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     2, 2,       _T("uaddc"),                              _T("uaddc")},
    { D3D11_SB_OPCODE_USUBB,                                     All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     2, 2,       _T("usubb"),                              _T("usubb")},
    { D3D11_SB_OPCODE_COUNTBITS,                                 All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("countbits"),                          _T("countbits")},
    { D3D11_SB_OPCODE_FIRSTBIT_HI,                               All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("firstbit_hi"),                        _T("firstbit_hi")},
    { D3D11_SB_OPCODE_FIRSTBIT_LO,                               All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("firstbit_lo"),                        _T("firstbit_lo")},
    { D3D11_SB_OPCODE_FIRSTBIT_SHI,                              All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("firstbit_shi"),                       _T("firstbit_shi")},
    { D3D11_SB_OPCODE_UBFE,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 3,       _T("ubfe"),                               _T("ubfe")},
    { D3D11_SB_OPCODE_IBFE,                                      All,         IT_Integer,     FALSE, FALSE, FALSE, DT_SInt,     1, 3,       _T("ibfe"),                               _T("ibfe")},
    { D3D11_SB_OPCODE_BFI,                                       All,         IT_Integer,     FALSE, FALSE, FALSE, DT_UInt,     1, 4,       _T("bfi"),                                _T("bfi")},
    { D3D11_SB_OPCODE_BFREV,                                     All,         IT_Integer,     FALSE, FALSE, FALSE, DT_Source0,  1, 1,       _T("bfrev"),                              _T("bfrev")},
    { D3D11_SB_OPCODE_SWAPC,                                     All,         IT_ALU,         FALSE, FALSE, FALSE, DT_Source1,  2, 3,       _T("swapc"),                              _T("swapc")},

    { D3D11_SB_OPCODE_DCL_STREAM,                                GS,          IT_Declaration, TRUE , FALSE, TRUE , DT_None,     1, 0,       _T("dcl_stream"),                         _T("dcl_stream")},
    { D3D11_SB_OPCODE_DCL_FUNCTION_BODY,                         All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_function_body"),                  _T("dcl_function_body")},
    { D3D11_SB_OPCODE_DCL_FUNCTION_TABLE,                        All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0xffff,  _T("dcl_function_table"),                 _T("dcl_function_table")},
    { D3D11_SB_OPCODE_DCL_INTERFACE,                             All,         IT_Declaration, FALSE, FALSE, FALSE, DT_None,     3, 0xffff,  _T("dcl_interface"),                      _T("dcl_interface dcl_interface_dynamicindexed")},

    { D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT,             All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     0, 0,       _T("dcl_input_control_point_count"),      _T("dcl_input_control_point_count")},
    { D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT,            All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     0, 0,       _T("dcl_output_control_point_count"),     _T("dcl_output_control_point_count")},
    { D3D11_SB_OPCODE_DCL_TESS_DOMAIN,                           All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     0, 0,       _T("dcl_tessellator_domain"),             _T("dcl_tessellator_domain")},
    { D3D11_SB_OPCODE_DCL_TESS_PARTITIONING,                     All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     0, 0,       _T("dcl_tessellator_partitioning"),       _T("dcl_tessellator_partitioning")},
    { D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE,                 All,         IT_Declaration, TRUE , FALSE, FALSE, DT_None,     0, 0,       _T("dcl_tessellator_output_primitive"),   _T("dcl_tessellator_output_primitive")},
    { D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR,                     HS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_hs_max_tessfactor"),              _T("dcl_hs_max_tessfactor")},
    { D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT,          HS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_hs_fork_phase_instance_count"),   _T("dcl_hs_fork_phase_instance_count")},
    { D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT,          HS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_hs_join_phase_instance_count"),   _T("dcl_hs_join_phase_instance_count")},

    { D3D11_SB_OPCODE_DCL_THREAD_GROUP,                          CS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     3, 0,       _T("dcl_thread_group"),                   _T("dcl_thread_group")},
    { D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED,           PS | CS,       IT_Declaration, TRUE , FALSE, TRUE,  DT_None,     3, 0,       _T("dcl_uav_typed"),                      _T("dcl_uav_typed dcl_uav_typed_buffer dcl_uav_typed_texture1d dcl_uav_typed_texture1darray dcl_uav_typed_texture2d dcl_uav_typed_texture2dms dcl_uav_typed_texture2darray dcl_uav_typed_texture2darrayms dcl_uav_typed_texture3d dcl_uav_typed_texturecube dcl_uav_typed_texturecubearray")},
    { D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW,             CS,          IT_Declaration, TRUE , FALSE, TRUE,  DT_None,     1, 0,       _T("dcl_uav_raw"),                        _T("dcl_uav_raw")},
    { D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED,      CS,          IT_Declaration, TRUE , FALSE, TRUE,  DT_None,     2, 0,       _T("dcl_uav_structured"),                 _T("dcl_uav_structured dcl_uav_structured_opc")},
    { D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW,        CS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     2, 0,       _T("dcl_tgsm_raw"),                       _T("dcl_tgsm_raw")},
    { D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED, CS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     3, 0,       _T("dcl_tgsm_structured"),                _T("dcl_tgsm_structured")},
    { D3D11_SB_OPCODE_DCL_RESOURCE_RAW,                          All,         IT_Declaration, TRUE , TRUE,  FALSE, DT_None,     1, 0,       _T("dcl_resource_raw"),                   _T("dcl_resource_raw")},
    { D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED,                   All,         IT_Declaration, TRUE , TRUE,  FALSE, DT_None,     2, 0,       _T("dcl_resource_structured"),            _T("dcl_resource_structured")},
    { D3D11_SB_OPCODE_LD_UAV_TYPED,                              All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 2,       _T("ld_uav_typed"),                       _T("ld_uav_typed")},
    { D3D11_SB_OPCODE_STORE_UAV_TYPED,                           All,         IT_Store,       FALSE, FALSE, FALSE, DT_Source1,  1, 2,       _T("store_uav_typed"),                    _T("store_uav_typed")},
    { D3D11_SB_OPCODE_LD_RAW,                                    All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 2,       _T("ld_raw"),                             _T("ld_raw")},
    { D3D11_SB_OPCODE_STORE_RAW,                                 CS,          IT_Store,       FALSE, FALSE, FALSE, DT_Source1,  1, 2,       _T("store_raw"),                          _T("store_raw")},
    { D3D11_SB_OPCODE_LD_STRUCTURED,                             All,         IT_Load,        FALSE, FALSE, FALSE, DT_Resource, 1, 3,       _T("ld_structured"),                      _T("ld_structured")},
    { D3D11_SB_OPCODE_STORE_STRUCTURED,                          CS,          IT_Store,       FALSE, FALSE, FALSE, DT_Source2,  1, 3,       _T("store_structured"),                   _T("store_structured")},
    { D3D11_SB_OPCODE_ATOMIC_AND,                                CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_and"),                         _T("atomic_and")},
    { D3D11_SB_OPCODE_ATOMIC_OR,                                 CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_or"),                          _T("atomic_or")},
    { D3D11_SB_OPCODE_ATOMIC_XOR,                                CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_xor"),                         _T("atomic_xor")},
    { D3D11_SB_OPCODE_ATOMIC_CMP_STORE,                          CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("atomic_cmp_store"),                   _T("atomic_cmp_store")},
    { D3D11_SB_OPCODE_ATOMIC_IADD,                               CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_iadd"),                        _T("atomic_iadd")},
    { D3D11_SB_OPCODE_ATOMIC_IMAX,                               CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_imax"),                        _T("atomic_imax")},
    { D3D11_SB_OPCODE_ATOMIC_IMIN,                               CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 2,       _T("atomic_imin"),                        _T("atomic_imin")},
    { D3D11_SB_OPCODE_ATOMIC_UMAX,                               CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 2,       _T("atomic_umax"),                        _T("atomic_umax")},
    { D3D11_SB_OPCODE_ATOMIC_UMIN,                               CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 2,       _T("atomic_umin"),                        _T("atomic_umin")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC,                          CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("imm_atomic_alloc"),                   _T("imm_atomic_alloc")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_CONSUME,                        CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 1,       _T("imm_atomic_consume"),                 _T("imm_atomic_consume")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_IADD,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_iadd"),                    _T("imm_atomic_iadd")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_AND,                            CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_and"),                     _T("imm_atomic_and")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_OR,                             CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_or"),                      _T("imm_atomic_or")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_XOR,                            CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_xor"),                     _T("imm_atomic_xor")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_EXCH,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Source2,  1, 3,       _T("imm_atomic_exch"),                    _T("imm_atomic_exch")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH,                       CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Source2,  1, 4,       _T("imm_atomic_cmp_exch"),                _T("imm_atomic_cmp_exch")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_IMAX,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_imax"),                    _T("imm_atomic_imax")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_IMIN,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_Int,      1, 3,       _T("imm_atomic_imin"),                    _T("imm_atomic_imin")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_UMAX,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 3,       _T("imm_atomic_umax"),                    _T("imm_atomic_umax")},
    { D3D11_SB_OPCODE_IMM_ATOMIC_UMIN,                           CS,          IT_Atomic,      FALSE, FALSE, FALSE, DT_UInt,     1, 3,       _T("imm_atomic_umin"),                    _T("imm_atomic_umin")},
    { D3D11_SB_OPCODE_SYNC,                                      CS,          IT_FlowControl, FALSE, FALSE, FALSE, DT_None,     0, 0,       _T("sync"),                               _T("sync_g sync_ugroup sync_uglobal sync_g_t sync_ugroup_t sync_uglobal_t sync_ugroup_g sync_uglobal_g sync_ugroup_g_t sync_uglobal_g_t")},

    { D3D11_SB_OPCODE_DADD,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 2,       _T("dadd"),                               _T("dadd dadd_sat")},
    { D3D11_SB_OPCODE_DMAX,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 2,       _T("dmax"),                               _T("dmax dmax_sat")},
    { D3D11_SB_OPCODE_DMIN,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 2,       _T("dmin"),                               _T("dmin dmin_sat")},
    { D3D11_SB_OPCODE_DMUL,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 2,       _T("dmul"),                               _T("dmul dmul_sat")},
    { D3D11_SB_OPCODE_DEQ,                                       All,         IT_Double,      FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("deq"),                                _T("deq")},
    { D3D11_SB_OPCODE_DGE,                                       All,         IT_Double,      FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("dge"),                                _T("dge")},
    { D3D11_SB_OPCODE_DLT,                                       All,         IT_Double,      FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("dlt"),                                _T("dlt")},
    { D3D11_SB_OPCODE_DNE,                                       All,         IT_Double,      FALSE, FALSE, FALSE, DT_SInt,     1, 2,       _T("dne"),                                _T("dne")},
    { D3D11_SB_OPCODE_DMOV,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 1,       _T("dmov"),                               _T("dmov dmov_sat")},
    { D3D11_SB_OPCODE_DMOVC,                                     All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 3,       _T("dmovc"),                              _T("dmovc dmov_satc")},
    { D3D11_SB_OPCODE_DTOF,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("dtof"),                               _T("dtof")},
    { D3D11_SB_OPCODE_FTOD,                                      All,         IT_Double,      FALSE, FALSE, FALSE, DT_Double,   1, 1,       _T("ftod"),                               _T("ftod")},

    { D3D11_SB_OPCODE_EVAL_SNAPPED,                              All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("eval_snapped"),                       _T("eval_snapped")},
    { D3D11_SB_OPCODE_EVAL_SAMPLE_INDEX,                         All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_Float,    1, 2,       _T("eval_sample_index"),                  _T("eval_sample_index")},
    { D3D11_SB_OPCODE_EVAL_CENTROID,                             All,         IT_FlowControl, FALSE, FALSE, FALSE, DT_Float,    1, 1,       _T("eval_centroid"),                      _T("eval_centroid")},

    { D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT,                     GS,          IT_Declaration, TRUE , FALSE, FALSE, DT_None,     1, 0,       _T("dcl_gs_instance_count"),              _T("dcl_gs_instance_count")},

    { D3D10_SB_NUM_OPCODES,                                      All,         IT_None,        FALSE, FALSE, FALSE, DT_Unknown,  0, 0,       _T("reserved3"),                          _T("")},
};

const DWORD g_dwOpCodeInfoCount = sizeof(g_OpCodeInfo) / sizeof(g_OpCodeInfo[0]);

const OpCodeInfo* D3D10ShaderUtils::GetOpCodeInfoTable()
{
    return g_OpCodeInfo;
}

const DWORD D3D10ShaderUtils::GetOpCodeInfoCount()
{
    return g_dwOpCodeInfoCount;
}

const OpCodeInfo* GetOpCodeInfo(D3D10_SB_OPCODE_TYPE opCode)
{
    for (DWORD i = 0; i < g_dwOpCodeInfoCount; i++)
    {
        if (g_OpCodeInfo[i].opCode == opCode)
        {
            return &g_OpCodeInfo[i];
        }
    }

    return NULL;
}

bool D3D10ShaderUtils::IsDCL(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->bIsDCL;
    }

    return false;
}

bool D3D10ShaderUtils::IsDCLInput(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->bIsDCLInput;
    }

    return false;
}

bool D3D10ShaderUtils::IsDCLOutput(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->bIsDCLOutput;
    }

    return false;
}

bool D3D10ShaderUtils::IsDCL_UAV(D3D10_SB_OPCODE_TYPE opCode)
{
    if (opCode == D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED ||
        opCode == D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW ||
        opCode == D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED)
    {
        return true;
    }

    return false;
}

bool D3D10ShaderUtils::IsInstruction(D3D10_SB_OPCODE_TYPE opCode)
{
    if (IsDCL(opCode) || opCode == D3D10_SB_OPCODE_LABEL || opCode == D3D10_SB_OPCODE_CUSTOMDATA ||
        opCode == D3D11_SB_OPCODE_HS_DECLS || opCode == D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE || opCode == D3D11_SB_OPCODE_HS_FORK_PHASE || opCode == D3D11_SB_OPCODE_HS_JOIN_PHASE)
    {
        return false;
    }

    return true;
}

bool D3D10ShaderUtils::IsFlowControlOp(D3D10_SB_OPCODE_TYPE opCode)
{
    switch (opCode)
    {
        case D3D10_SB_OPCODE_BREAK:
        case D3D10_SB_OPCODE_BREAKC:
        case D3D10_SB_OPCODE_CALL:
        case D3D10_SB_OPCODE_CALLC:
        case D3D10_SB_OPCODE_CASE:
        case D3D10_SB_OPCODE_CONTINUE:
        case D3D10_SB_OPCODE_CONTINUEC:
        case D3D10_SB_OPCODE_DEFAULT:
        case D3D10_SB_OPCODE_ELSE:
        case D3D10_SB_OPCODE_ENDIF:
        case D3D10_SB_OPCODE_ENDLOOP:
        case D3D10_SB_OPCODE_ENDSWITCH:
        case D3D10_SB_OPCODE_IF:
        case D3D10_SB_OPCODE_LABEL:
        case D3D10_SB_OPCODE_LOOP:
        case D3D10_SB_OPCODE_RET:
        case D3D10_SB_OPCODE_RETC:
        case D3D10_SB_OPCODE_SWITCH:
            return true;

        default:
            return false;
    }
}

bool D3D10ShaderUtils::HasDestReg(D3D10_SB_OPCODE_TYPE opCode)
{
    if (GetDestRegCount(opCode) >= 1)
    {
        return true;
    }

    return false;
}

bool D3D10ShaderUtils::IsTwoDestRegInst(D3D10_SB_OPCODE_TYPE opCode)
{
    if (GetDestRegCount(opCode) == 2)
    {
        return true;
    }

    return false;
}

DWORD D3D10ShaderUtils::GetDestRegCount(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->dwDestRegCount;
    }

    return 0;
}

DWORD D3D10ShaderUtils::GetSrcRegCount(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->dwSrcRegCount;
    }

    return 0;
}

DWORD GetNumComponents(D3D10_SB_OPERAND_NUM_COMPONENTS opNumComponents)
{
    switch (opNumComponents)
    {
        case D3D10_SB_OPERAND_0_COMPONENT:  return 0;

        case D3D10_SB_OPERAND_1_COMPONENT:  return 1;

        case D3D10_SB_OPERAND_4_COMPONENT:  return 4;

        case D3D10_SB_OPERAND_N_COMPONENT:
        default:                            return 0;
    }
}

DWORD D3D10ShaderUtils::GetOperandLength(DWORD* pdwToken)
{
    DWORD dwSrcRegLength = 1; // Base operand token
    DWORD dwToken = *pdwToken;
    D3D10_SB_OPERAND_TYPE operandType = DECODE_D3D10_SB_OPERAND_TYPE(dwToken);

    // Immediate operands
    DWORD dwNumComponents = GetNumComponents(DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(dwToken));

    switch (operandType)
    {
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:   dwSrcRegLength += dwNumComponents; pdwToken += dwNumComponents; break;

        case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:   dwSrcRegLength += (2 * dwNumComponents); pdwToken += (2 * dwNumComponents); break;

        default: break;
    }

    if (DECODE_IS_D3D10_SB_OPERAND_EXTENDED(dwToken))
    {
        dwSrcRegLength++;
        pdwToken++;
    }

    DWORD dwNumIndices = DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(dwToken);

    for (DWORD i = 0; i < dwNumIndices; i++)
    {
        D3D10_SB_OPERAND_INDEX_REPRESENTATION indexType = DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, dwToken);

        switch (indexType)
        {
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:  dwSrcRegLength++; pdwToken++; break;

            case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:  dwSrcRegLength += 2; pdwToken += 2; break;

            case D3D10_SB_OPERAND_INDEX_RELATIVE:
            {
                DWORD dwSubOperandLength = GetOperandLength(pdwToken + 1);
                dwSrcRegLength += dwSubOperandLength;
                pdwToken += dwSubOperandLength;
                break;
            }

            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
            {
                //Immediate
                dwSrcRegLength++;
                pdwToken++;

                // Relative
                DWORD dwSubOperandLength = GetOperandLength(pdwToken + 1);
                dwSrcRegLength += dwSubOperandLength;
                pdwToken += dwSubOperandLength;
                break;
            }

            case D3D10_SB_OPERAND_INDEX_IMMEDIATE64_PLUS_RELATIVE:
            {
                //Immediate
                dwSrcRegLength += 2;
                pdwToken += 2;

                // Relative
                DWORD dwSubOperandLength = GetOperandLength(pdwToken + 1);
                dwSrcRegLength += dwSubOperandLength;
                pdwToken += dwSubOperandLength;
                break;
            }
        }
    }

    return dwSrcRegLength;
}

DWORD D3D10ShaderUtils::GetComponentMask(D3D10_SB_4_COMPONENT_NAME component)
{
    switch (component)
    {
        case D3D10_SB_4_COMPONENT_X:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_X;

        case D3D10_SB_4_COMPONENT_Y:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;

        case D3D10_SB_4_COMPONENT_Z:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;

        case D3D10_SB_4_COMPONENT_W:  return D3D10_SB_OPERAND_4_COMPONENT_MASK_W;

        default:                      return 0;
    }
}

const TCHAR* D3D10ShaderUtils::GetOpCodeName(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->pszName;
    }

    return _T("Unknown OpCode");
}

DestType D3D10ShaderUtils::GetOpCodeDestType(D3D10_SB_OPCODE_TYPE opCode)
{
    const OpCodeInfo* pOpCodeInfo = GetOpCodeInfo(opCode);

    if (pOpCodeInfo != NULL)
    {
        return pOpCodeInfo->eDestType;
    }

    return DT_Unknown;
}

DestType D3D10ShaderUtils::GetOpCodeDestType(DWORD dwOpCodeToken)
{
    D3D10_SB_OPCODE_TYPE opCode = DECODE_D3D10_SB_OPCODE_TYPE(dwOpCodeToken);
    DestType destType = GetOpCodeDestType(opCode);

    if (destType == DT_Encoded)
    {
        if (opCode == D3D10_SB_OPCODE_RESINFO || opCode == D3D11_SB_OPCODE_BUFINFO)
        {
            if (DECODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(dwOpCodeToken) == D3D10_SB_RESINFO_INSTRUCTION_RETURN_UINT)
            {
                return DT_UInt;
            }
            else
            {
                return DT_Float;
            }
        }
        else
        {
            if (DECODE_D3D10_SB_INSTRUCTION_RETURN_TYPE(dwOpCodeToken) == D3D10_SB_INSTRUCTION_RETURN_UINT)
            {
                return DT_UInt;
            }
            else
            {
                return DT_Float;
            }
        }
    }

    return destType;
}

DestType D3D10ShaderUtils::GetDestType(D3D10_RESOURCE_RETURN_TYPE resReturnType)
{
    switch (resReturnType)
    {
        case D3D10_RETURN_TYPE_SINT:  return DT_SInt;

        case D3D10_RETURN_TYPE_UINT:  return DT_UInt;

        default:                      return DT_Float;
    }
}

bool D3D10ShaderUtils::IsComputeBuffer(D3D10_SHADER_INPUT_TYPE inputType)
{
    switch (inputType)
    {
        case D3D10_SIT_TEXTURE:                         // Fall-through
        case D3D10_SIT_TBUFFER:                         // Fall-through
        case D3D10_SIT_SAMPLER:                         return false;

        case D3D11_SIT_UAV_RWTYPED:                     // Fall-through
        case D3D11_SIT_STRUCTURED:                      // Fall-through
        case D3D11_SIT_UAV_RWSTRUCTURED:                // Fall-through
        case D3D11_SIT_BYTEADDRESS:                     // Fall-through
        case D3D11_SIT_UAV_RWBYTEADDRESS:               // Fall-through
        case D3D11_SIT_UAV_APPEND_STRUCTURED:           // Fall-through
        case D3D11_SIT_UAV_CONSUME_STRUCTURED:          // Fall-through
        case D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:   return true;

        default:                                        return false;
    }
}
