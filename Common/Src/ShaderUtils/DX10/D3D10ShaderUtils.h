//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10ShaderUtils.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10ShaderUtils.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#pragma once

#ifndef D3D10SHADERUTILS_H
#define D3D10SHADERUTILS_H

#include <d3d10_1.h>
#include "d3d11tokenizedprogramformat.hpp"
#include <tchar.h>
#include <string>

/// D3D10ShaderUtils is a set of D3D10 shader utility functions.
namespace D3D10ShaderUtils
{
/// The D3D10 shader model.
typedef enum
{
    SM_4_0 = 0x40, ///< Shader Model 4.0 (aka D3D10).
    SM_4_1 = 0x41, ///< Shader Model 4.1 (aka D3D10.1).
    SM_5_0 = 0x50, ///< Shader Model 5.0 (aka D3D11).
} ShaderModel;

/// The D3D10 shader type.
typedef enum
{
    None = 0x00,   ///< Unknown shader type.
    PS   = 0x01,   ///< Pixel shader.
    VS   = 0x02,   ///< Vertex shader.
    GS   = 0x04,   ///< Geometry shader.
    HS   = 0x08,   ///< Hull shader.
    DS   = 0x10,   ///< Domain shader.
    CS   = 0x20,   ///< Compute shader.
} ShaderType;

/// Bitfield mask for all available shader types.
static const UCHAR All = PS | VS | GS | HS | DS | CS;

/// The type of an instruction.
typedef enum
{
    IT_None,          ///< Unknown instruction type.
    IT_Declaration,   ///< Declaration instruction.
    IT_ALU,           ///< ALU instruction.
    IT_Integer,       ///< Integer ALU instruction.
    IT_Double,        ///< Double float ALU instruction.
    IT_FlowControl,   ///< Flow-control instruction.
    IT_Load,          ///< Load instruction.
    IT_Store,         ///< Store instruction.
    IT_Atomic,        ///< Atomic instruction.
    IT_Other,         ///< Other instruction type.
} InstructionType;

/// The types of destination register contents.
typedef enum
{
    DT_Unknown,    ///< Unknown destination type.
    DT_None,       ///< No destination.
    DT_Resource,   ///< Based on the format of the resource being accessed, the resource reg being source 2.
    DT_Encoded,    ///< The format is encoded in token.
    DT_Source0,    ///< The format of source register 0, taking into account modifiers.
    DT_Source1,    ///< The format of source register 1, taking into account modifiers.
    DT_Source2,    ///< The format of source register 2, taking into account modifiers.
    DT_Half,       ///< Half floating point (16 bits).
    DT_Float,      ///< Floating point (32 bits).
    DT_Double,     ///< Double floating point (64 bits).
    DT_Int,        ///< Could be signed or unsigned int depending on the input type.
    DT_UInt,       ///< Unsigned integer.
    DT_SInt,       ///< Signed integer.
} DestType;

/// Information describing a D3D10 shader instruction.
typedef struct
{
    D3D10_SB_OPCODE_TYPE opCode;           ///< The op-code of the instruction.
    UCHAR                shaderTypes;      ///< A bit-field of D3D10ShaderUtils::ShaderType values describing the types of shader that support this opcode.
    InstructionType      instructionType;  ///< The type of instruction.
    bool                 bIsDCL;           ///< Is the instruction a Declaration?
    bool                 bIsDCLInput;      ///< Is the instruction an Input Declaration?
    bool                 bIsDCLOutput;     ///< Is the instruction an Output Declaration?
    DestType             eDestType;        ///< The type of data the instruction outputs.
    DWORD                dwDestRegCount;   ///< The number of outputs from the instruction.
    DWORD                dwSrcRegCount;    ///< The number of source registers for the instruction.
    const TCHAR*         pszName;          ///< The name of the instruction.
    const TCHAR*         pszKeyWords;      ///< The possible set of keywords for the instruction.
} OpCodeInfo;

/// Get a pointer to the op-code info table.
/// \return The address of the op-code info table.
const OpCodeInfo* GetOpCodeInfoTable();

/// Get the number of entries in the op-code info table.
/// \return The number of entries in the op-code info table.
const DWORD GetOpCodeInfoCount();

/// Is the op-code a DCL op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL op-code, otherwise false.
bool IsDCL(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code a DCL_INPUT op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_INPUT op-code, otherwise false.
bool IsDCLInput(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code a DCL_OUTPUT op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_OUTPUT op-code, otherwise false.
bool IsDCLOutput(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code a DCL_UAV op-code?
/// \param[in] opCode The op-code.
/// \return True if a v op-code, otherwise false.
bool IsDCL_UAV(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code an instruction?
/// \param[in] opCode The op-code.
/// \return True if an instruction, otherwise false.
bool IsInstruction(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code a flow-control instruction?
/// \param[in] opCode The op-code.
/// \return True if a flow-control instruction, otherwise false.
bool IsFlowControlOp(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code an instruction that has a destination register?
/// \param[in] opCode The op-code.
/// \return True if an instruction that has a destination register, otherwise false.
bool HasDestReg(D3D10_SB_OPCODE_TYPE opCode);

/// Is the op-code an instruction that has two destination registers?
/// \param[in] opCode The op-code.
/// \return True if an instruction that has two destination registers, otherwise false.
bool IsTwoDestRegInst(D3D10_SB_OPCODE_TYPE opCode);

/// How many destination registers does this op-code use?
/// This includes dest registers that are null.
/// \param[in] opCode The op-code.
/// \return The number of destination registers used by this op-code.
DWORD GetDestRegCount(D3D10_SB_OPCODE_TYPE opCode);

/// How many source registers does this op-code use?
/// \param[in] opCode The op-code.
/// \return The number of source registers used by this op-code.
DWORD GetSrcRegCount(D3D10_SB_OPCODE_TYPE opCode);

/// What is the length of an operand in DWORDS?
/// \param[in] pdwToken A pointer to the operand
/// \return The length of the operand in DWORDS.
DWORD GetOperandLength(DWORD* pdwToken);

/// Retrieve the component mask for the specified component?
/// \param[in] component The component name.
/// \return The component mask.
DWORD GetComponentMask(D3D10_SB_4_COMPONENT_NAME component);

/// Get the op-code name.
/// \param[in] opCode The op-code.
/// \return The op-code name.
const TCHAR* GetOpCodeName(D3D10_SB_OPCODE_TYPE opCode);

/// Get the op-code dest type.
/// \param[in] opCode The op-code.
/// \return The op-code dest type.
DestType GetOpCodeDestType(D3D10_SB_OPCODE_TYPE opCode);

/// Get the op-code dest type.
/// \param[in] dwOpCodeToken The op-code token.
/// \return The op-code dest type.
DestType GetOpCodeDestType(DWORD dwOpCodeToken);

/// Get the dest type.
/// \param[in] resReturnType The resource return type.
/// \return The dest type.
DestType GetDestType(D3D10_RESOURCE_RETURN_TYPE resReturnType);

/// Is the shader input type a compute buffer.
/// \param[in] inputType The shader input type.
/// \return true if the shader input type is a compute buffer, otherwise false.
bool IsComputeBuffer(D3D10_SHADER_INPUT_TYPE inputType);
}; // D3D10ShaderUtils


#endif // D3D10SHADEROBJECT_H