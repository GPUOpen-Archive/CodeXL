//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACES_OCL_H_
#define _INTERFACES_OCL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
//! OpenCL API interfaces
typedef oclElf*
( AOC_INTERFACE *CompileSource)(
    oclCompiler* cl, 
    const char** source, 
    size_t num_sources,
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

typedef oclElf*
( AOC_INTERFACE *CompileSourceToIR)(
    oclCompiler* cl, 
    const char** source, 
    size_t num_sources,
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

typedef oclElf*
( AOC_INTERFACE *CompileIRToIL)(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;


typedef oclElf*
( AOC_INTERFACE *CompileILToISA)(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;


typedef oclElf*
( AOC_INTERFACE *CompileBinary)(
    oclCompiler* cl, 
    const oclElf* binary, 
    const char* options, 
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

typedef const char*
( AOC_INTERFACE *GetCompilerLog)(
    oclCompiler* cl
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *InsertSection)(
    oclCompiler *cl,
    oclElf* binary, 
    const void* blob, 
    size_t size,
    unsigned name
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *ExtractSection)(
    oclCompiler *cl,
    const oclElf* binary,
    size_t* size,
    unsigned name
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *InsertSymbol)(
    oclCompiler *cl,
    oclElf* binary, 
    const void* blob, 
    size_t size,
    unsigned name,
    const char* symbol
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *ExtractSymbol)(
    oclCompiler *cl,
    const oclElf* binary,
    size_t* size,
    unsigned name,
    const char* symbol
    ) AOC_API_0_7;
#endif // _INTERFACES_OCL_H_
