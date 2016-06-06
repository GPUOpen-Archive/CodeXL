//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_HSAIL_H_
#define _API_HSAIL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"
#ifdef __cplusplus
extern "C" {
#endif
//! The function oclhsaCompileHSAIL takes as an argument a HSAIL source 
// string to compile and the options to compiler. The third argument is the 
// oclTargetInfo object that specifies what the target is. The fourth 
// argument is a callback function that gets the compiler warnings/errors.
// The result is a oclElf binary if no errors occured, NULL otherwise.
oclElf* AOC_INTERFACE 
oclhsaCompileSource(
    oclCompiler* cl,
    const char* source,
    const char* options,
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclhsaCompile takes an oclElf binary along with kernel name
// and options and tells SC to compile them down to ISA. If the compilation
// is successful, the ISA is stored in the oclElf binary. How this exists in
// the oclElf binary is specified in the BIF document. If any options are
// invalid for the device, they are ignored. Any options can be ignored or
// overridden by the compiler, but doing so must produce a warning message
// in the error log. If the kernel does not exist in the binary as HSAIL, but exists in a higher level form, recompilation from a higher level to HSAIL is
// required. Error and warning messages are only to be emitted if error_log
// exists.
bool AOC_INTERFACE 
oclhsaCompileBinary(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* options, 
    const char* kernel, 
    oclLogFunction error_log
    ) AOC_API_0_7;

//! The function oclhsaSetType allows the explicit insertion of an HSAIL 
// Text/Binary blob for a given kernel into an oclElf binary. 
bool AOC_INTERFACE 
oclhsaSetType(
    oclCompiler* cl,
    oclElf* bin, 
    const char* kernel, 
    const void* HSAIL, 
    size_t size, 
    oclhsaType type
    ) AOC_API_0_7;

//! The function oclhsaRetrieveType returns the HSAIL in the binary based on the
// type argument. The pointer is safe to cast to a const char* if type is
// equal to oclhsaText and the value returned is non-NULL. The length of the
// blob/text is stored in the size argument. If the kernel does not exist in
// the binary or the binary is invalid, NULL is returned. If the oclhsaType
// is oclhsaText and only the IL binary form exists, the form must be
// converted to the appropriate format before returning the HSAIL in the
// correct format. The opposite also applies.
const void* AOC_INTERFACE 
oclhsaRetrieveType(
    oclCompiler* cl, 
    const oclElf* bin, 
    const char* kernel, 
    size_t* size, 
    oclhsaType type
    ) AOC_API_0_7;

//! The function oclhsaConvertType converts the kernel in the oclElf binary from
// the type specified in the from field to its opposite type. If oclhsaText is
// specified, the HSAIL that matches the kernel is converted from HSAIL Text
// to HSAIL Binary and vice versa if oclhsaBinary is specified. If the kernel
// does not exist in the binary  an error is returned. If the kernel exists
// but the type specified is oclhsaText and a higher level version of the
// kernel exists, the HSAIL is to be regenerated before converting to
// oclhsaBinary.
bool AOC_INTERFACE 
oclhsaConvertType(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel, 
    oclhsaType from
    ) AOC_API_0_7;
#ifdef __cplusplus
}
#endif
#endif // _API_HSAIL_H_
