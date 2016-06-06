//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACE_API_H_
#define _INTERFACE_API_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
typedef bool
( AOC_INTERFACE *irSetType)(
    oclCompiler* cl,
    oclElf* bin, 
    const void* IR, 
    size_t size, 
    oclirType type
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *irRetrieveType)(
    oclCompiler* cl,
    const oclElf* bin, 
    size_t* size, 
    oclirType type
    ) AOC_API_0_7;

typedef bool
( AOC_INTERFACE *irLink)(
    oclCompiler* cl,
    const oclElf* bin, 
    size_t** size, 
    const void **modules,
    size_t num, 
    oclLogFunction error_log
    ) AOC_API_0_7;
#endif // _INTERFACE_API_H_
