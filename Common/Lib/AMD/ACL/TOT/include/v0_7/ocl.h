//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_OCL_H_
#define _API_OCL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"
#ifdef __cplusplus
extern "C" {
#endif
//! The function oclCompileSource takes as an argument the CL source to
// compile and the options to compiler. The fifth argument is the oclTargetInfo
// object that specifies what the target is. The sixth argument is a callback
// function that gets the compiler warnings/errors. The result is a oclElf
// binary if no errors occured, NULL otherwise.
oclElf* AOC_INTERFACE
oclCompileSource(
    oclCompiler* cl, 
    const char** source, 
    size_t num_sources,
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclCompileSourceToIR takes as an argument the CL source to
// compile and the options to compiler. The fifth argument is the oclTargetInfo
// object that specifies what the target is. The sixth argument is a callback
// function that gets the compiler warnings/errors. The result is a oclElf
// binary if no errors occured, NULL otherwise. This function only compiles from
// the source language to the internal IR representation of the compiler.
oclElf* AOC_INTERFACE 
oclCompileSourceToIR(
    oclCompiler* cl, 
    const char** source, 
    size_t num_sources,
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclCompileIRToIL takes as an argument an OpenCL binary to
// compile and the options to compiler. The fourth argument is the oclTargetInfo
// object that specifies what the target is. The fifth argument is a callback
// function that gets the compiler warnings/errors. The result is a oclElf 
// binary if no errors occured, NULL otherwise. This fucntion compiles from
// the internal IR representation of the compiler to the target specific
// IL representation. If you want to compile down to ISA directly from
// the IR, use oclCompileBinary instead.
oclElf* AOC_INTERFACE 
oclCompileIRToIL(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclCompileILToISA takes as an argument an OpenCL binary to
// compile and the options to compiler. The fourth argument is the oclTargetInfo
// object that specifies what the target is. The fifth argument is a callback
// function that gets the compiler warnings/errors. The result is a oclElf 
// binary if no errors occured, NULL otherwise. This function compiles from the
// target generic IL to the device specific ISA.
oclElf* AOC_INTERFACE 
oclCompileILToISA(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclCompileBinary takes as an argument an OpenCL binary to
// compile and the options to compiler. The fourth argument is the oclTargetInfo
// object that specifies what the target is. The fifth argument is a callback
// function that gets the compiler warnings/errors. The result is a oclElf 
// binary if no errors occured, NULL otherwise. 
oclElf* AOC_INTERFACE 
oclCompileBinary(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

//! The function oclGetCompilerLog returns a NULL terminated C-style string 
// that holds the compiler log from the result of oclCompileSource or 
// oclCompileBinary. If no log exists, NULL is returned. The returned
// string should NOT be deallocated.
const char* AOC_INTERFACE 
oclGetCompilerLog(oclCompiler* CL) AOC_API_0_7;

//! The elfInsertSection function inserts a binary blob into the oclElf 
// binary the section specified by the type argument. The size of the blob is
// specified by the size argument. If the insertion succeeds, the function 
// returns true.
bool AOC_INTERFACE 
elfInsertSection(
    oclCompiler *cl,
    oclElf* binary, 
    const void* blob, 
    size_t size,
    unsigned name
    ) AOC_API_0_7;

//! The elfExtractSection function extracts a binary blob from the oclElf
//binary and returns the size in the size paramter. The section is specified
//by the name field
const void* AOC_INTERFACE 
elfExtractSection(
    oclCompiler *cl,
    const oclElf* binary,
    size_t* size,
    unsigned name
    ) AOC_API_0_7;

//! The elfInsertSymbol function inserts a binary blob into the oclElf 
// binary the section specified by the type argument at the specified symbol. 
// The size of the blob is specified by the size argument. If the insertion 
// succeeds, the function returns true.
bool AOC_INTERFACE 
elfInsertSymbol(
    oclCompiler *cl,
    oclElf* binary, 
    const void* blob, 
    size_t size,
    unsigned name,
    const char* symbol
    ) AOC_API_0_7;

//! The elfExtractSymbol function extracts a binary blob from the oclElf
//binary and returns the size in the size paramter. The section is specified
//by the name field and the symbol specifies what to take out of the section.
const void* AOC_INTERFACE 
elfExtractSymbol(
    oclCompiler *cl,
    const oclElf* binary,
    size_t* size,
    unsigned name,
    const char* symbol
    ) AOC_API_0_7;
#ifdef __cplusplus
}
#endif
#endif // _API_OCL_H_
