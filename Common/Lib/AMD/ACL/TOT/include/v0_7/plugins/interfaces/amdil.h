//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACE_AMDIL_H_
#define _INTERFACE_AMDIL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"

typedef oclElf*
( AOC_INTERFACE *ilCompileSource)(
    oclCompiler* cl, 
    const char* source, 
    const char* options, 
    const oclTargetInfo* target, 
    oclLogFunction compile_callback
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *ilCompileBinary)(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* options, 
    const char* kernel, 
    oclLogFunction error_log
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *ilSetType)(
    oclCompiler* cl,
    oclElf* bin, 
    const char* kernel, 
    const void* IL, 
    size_t size, 
    oclilType type
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *ilRetrieveType)(
    oclCompiler* cl, 
    const oclElf* bin, 
    const char* kernel, 
    size_t* size, 
    oclilType type
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *ilConvertType)(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel,
    oclilType from
    ) AOC_API_0_7;
#endif // _INTERFACE_AMDIL_H_
