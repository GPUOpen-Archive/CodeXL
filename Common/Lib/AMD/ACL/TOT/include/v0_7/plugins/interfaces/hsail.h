//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACES_HSAIL_H_
#define _INTERFACES_HSAIL_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
typedef oclElf*
( AOC_INTERFACE *hsaCompileSource)(
    oclCompiler* cl,
    const char* source,
    const char* options,
    const oclTargetInfo* target,
    oclLogFunction compile_callback
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *hsaCompileBinary)(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* options, 
    const char* kernel, 
    oclLogFunction error_log
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *hsaSetType)(
    oclCompiler* cl,
    oclElf* bin, 
    const char* kernel, 
    const void* HSAIL, 
    size_t size, 
    oclhsaType type
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *hsaRetrieveType)(
    oclCompiler* cl, 
    const oclElf* bin, 
    const char* kernel, 
    size_t* size, 
    oclhsaType type
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *hsaConvertType)(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel, 
    oclhsaType from
    ) AOC_API_0_7;
#endif // _INTERFACES_HSAIL_H_
