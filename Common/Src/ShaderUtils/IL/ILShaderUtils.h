//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ILShaderUtils.h
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/IL/ILShaderUtils.h#11 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef ILSHADERUTILS_H
#define ILSHADERUTILS_H

#include <string>

#include "ILTables.h"
#include "ILFormat.h"

// define this type ourselves
typedef unsigned int IL;


/// ILShaderUtils is a set of IL shader utility functions.
namespace ILShaderUtils
{
/// Is the op-code a DCL op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL op-code, otherwise false.
bool IsDCL(ILOpCode opCode);

/// Is the op-code a DCL_INPUT op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_INPUT op-code, otherwise false.
bool IsDCLInput(ILOpCode opCode);

/// Is the op-code a DCL_OUTPUT op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_OUTPUT op-code, otherwise false.
bool IsDCLOutput(ILOpCode opCode);

/// Is the op-code a DCL_LITERAL op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_LITERAL op-code, otherwise false.
bool IsDCLLiteral(ILOpCode opCode);

/// Is the op-code a DCL_UAV op-code?
/// \param[in] opCode The op-code.
/// \return True if a DCL_UAV op-code, otherwise false.
bool IsDCL_UAV(ILOpCode opCode);

/// Is the op-code an instruction?
/// \param[in] opCode The op-code.
/// \return True if an instruction, otherwise false.
bool IsInstruction(ILOpCode opCode);

/// Is the op-code a flow-control instruction?
/// \param[in] opCode The op-code.
/// \return True if a flow-control instruction, otherwise false.
bool IsFlowControlOp(ILOpCode opCode);

/// Is the op-code a global store instruction?
/// \param[in] opCode The op-code.
/// \return True if a store instruction, otherwise false.
bool IsGlobalStore(ILOpCode opCode);

/// Is the op-code a global load instruction?
/// \param[in] opCode The op-code.
/// \return True if a load instruction, otherwise false.
bool IsGlobalLoad(ILOpCode opcode);

/// Is the op-code a global memory access instruction?
/// \param[in] opCode The op-code.
/// \return True if a global memory access instruction, otherwise false.
bool IsGlobalMemAccessOp(ILOpCode code);

/// Is the op-code a global memory atomic instruction?
/// \param[in] opCode The op-code.
/// \return True if an atomic instruction, otherwise false.
bool IsGlobalAtomicOp(ILOpCode opcode);

/// Is the op-code a sample instruction?
/// \param[in] opCode The op-code.
/// \return True if an sample instruction, otherwise false.
bool IsSampleOp(ILOpCode opcode);

/// Is the op-code an fetch4 instruction?
/// \param[in] opCode The op-code.
/// \return True if an fetch4 instruction, otherwise false.
bool IsFetch4Op(ILOpCode opcode);

/// Is the op-code an atomic operation that returns a value?
/// \param[in] opCode The op-code.
/// \return True if an atomic operation that returns a value, otherwise false.
bool IsReadAtomicOp(ILOpCode opcode);

/// Is the op-code a flow-control instruction that starts a block of code?
/// \param[in] opCode The op-code.
/// \return True if flow-control instruction starts block of code, otherwise false.
bool IsFlowControlBlockOp(ILOpCode opCode);

/// Does the op-code use a second token as an instruction modifier?
/// \param[in] opCode The op-code.
/// \return True if the instruction has a modifier, otherwise false
bool HasInstructionModifier(ILOpCode opCode);

/// Does the op-code use the pri and sec modifier bits as usual?
/// \param[in] opCode The op-code.
/// \return True if the instruction honours pri and sec modifiers,
/// false if these bits are used for soemthing else
bool UsesStandardModifiers(ILOpCode opCode);

/// Is the op-code an instruction that has a destination register?
/// \param[in] opCode  The op-code.
/// \param[in] mod     Modifier token. Only used for MDEF and MCALL.
/// \return True if an instruction that has a destination register, otherwise false.
bool HasDestReg(IL_OpCode op,
                IL mod);

/// How many destination registers does this op-code use?
/// This includes dest registers that are null.
/// \param[in] opCode  The op-code.
/// \param[in] mod     Modifier token. Only used for MDEF and MCALL.
/// \return The number of destination registers used by this op-code.
unsigned int GetDestRegCount(IL_OpCode op,
                             IL mod);

/// How many source registers does this op-code use?
/// \param[in] opCode  The op-code.
/// \param[in] mod Modifier token. Only used for MCALL.
/// \return The number of source registers used by this op-code.
unsigned int GetSrcRegCount(IL_OpCode op,
                            IL mod);

/// What is the length of an operand in DWORDS?
/// \param[in] pdwToken A pointer to the operand
/// \return The length of the operand in DWORDS.
//int GetOperandLength( IL* pdwToken );

/// Retrieve the component mask for the specified component
/// \param[in] component The component name.
/// \return The component mask.
//DWORD GetComponentMask( D3D10_SB_4_COMPONENT_NAME component );

/// Get the op-code name.
/// \param[in] opCode The op-code.
/// \return The op-code name.
std::string GetOpCodeName(ILOpCode opCode);

/// Get the dest type.
/// \param[in] dst The destination.
/// \return The dest type.
ILRegType GetOpCodeDestType(IL_Dst dst);

/// Get the dest type.
/// \param[in] token The destination token.
/// \return The dest type.
ILRegType GetOpCodeDestType(IL token);

/// Get the source type.
/// \param[in] src The source.
/// \return The source type.
ILRegType GetOpCodeSourceType(IL_Src src);

/// Get the source type.
/// \param[in] token The source token.
/// \return The source type.
ILRegType GetOpCodeSourceType(IL token);

}; // ILShaderUtils


#endif // ILSHADEROBJECT_H
