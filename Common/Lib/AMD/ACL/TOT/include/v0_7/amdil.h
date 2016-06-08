//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_AMDIL_H_
#define _API_AMDIL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"

#ifdef __cplusplus
extern "C" {
#endif
//! The function oclilCompileSource takes as an argument a AMDIL source string
// to compile and the options to compiler. The third argument is the
// oclTargetInfo object that specifies what the target is. The fourth
// argument is a callback function that gets the compiler warnings/errors.
// The result is a oclElf binary if no errors occured, NULL otherwise.
oclElf* AOC_INTERFACE 
oclilCompileSource(
    oclCompiler* cl, 
    const char* source, 
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclilCompileBinary takes an oclElf binary along with kernel
// name and options and tells SC to compile them down to ISA. If the
// compilation is successful, the ISA is stored in the oclElf binary. How
// this exists in the oclElf binary is specified in the BIF document. If any
// options are invalid for the device, they are ignored. Any options can be
// ignored or overridden by the compiler, but doing so must produce a warning
// message in the error log. If the kernel does not exist in the binary as
// AMDIL, but exists in a higher level form, recompilation from a higher level
// to IL is required. Error and warning messages are only to be emitted if
// error_log exists.
bool AOC_INTERFACE 
oclilCompileBinary(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* options, 
    const char* kernel, 
    oclLogFunction error_log
    ) AOC_API_0_7;

//! The function oclilSetType allows the explicit insertion of an IL
// Text/Binary blob for a given kernel into an oclElf binary. 
bool AOC_INTERFACE 
oclilSetType(
    oclCompiler* cl,
    oclElf* bin, 
    const char* kernel, 
    const void* IL, 
    size_t size, 
    oclilType type
    ) AOC_API_0_7;

//! The function oclilRetrieveType returns the AMDIL in the binary based on
// the type argument. The pointer is safe to cast to a const char* if type is
// equal to oclilText and the value returned is non-NULL. The length of the
// blob/text is stored in the size argument. If the kernel does not exist in
// the binary or the binary is invalid, NULL is returned. If the oclilType is
// oclilText and only the AMDIL binary form exists, the form must be converted
// to the appropriate format before returning the AMDIL in the correct format.
// The opposite also applies.
const void* AOC_INTERFACE 
oclilRetrieveType(
    oclCompiler* cl, 
    const oclElf* bin, 
    const char* kernel, 
    size_t* size, 
    oclilType type
    ) AOC_API_0_7;

//! The function oclilConvert converts the kernel in the oclElf binary from
// the type specified in the from field to its opposite type. If oclilText is
// specified, the AMDIL that matches the kernel is converted from AMDIL Text
// to AMDIL Binary and vice versa if oclilBinary is specified. If the kernel
// does not exist in the binary  an error is returned. If the kernel exists
// but the type specified is oclilText and a higher level version of the kernel
// exists, the IL is to be regenerated before converting to oclilBinary.
bool AOC_INTERFACE 
oclilConvertType(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel,
    oclilType from
    ) AOC_API_0_7;
#ifdef __cplusplus
}
#endif

#endif // _API_AMDIL_H_

