//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACE_GPUISA_H_
#define _INTERFACE_GPUISA_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
typedef void
( AOC_INTERFACE *isaDisassemble)(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel, 
    oclLogFunction disasm_callback
    ) AOC_API_0_7;

typedef const void*
( AOC_INTERFACE *isaGetBinaryBlob)(
    oclCompiler* cl,
    const oclElf* bin, 
    const char* kernel, 
    size_t* size
    ) AOC_API_0_7;
#endif // _INTERFACE_GPUISA_H_
