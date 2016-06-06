//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_LLVM_H_
#define _API_LLVM_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"

#ifdef __cplusplus
extern "C" {
#endif
//! The function oclirSetType allows the explicit insertion of an LLVM-IR
// blob for a given kernel into an oclElf binary. 
bool AOC_INTERFACE 
oclirSetType(
    oclCompiler* cl,
    oclElf* bin, 
    const void* IR, 
    size_t size, 
    oclirType type
    ) AOC_API_0_7;

//! The function oclirRetrieveType returns the IR in the binary based on the
// type argument. The length of the blob is stored in the size argument. 
const void* AOC_INTERFACE  
oclirRetrieveType (
    oclCompiler* cl,
    const oclElf* bin, 
    size_t* size, 
    oclirType type
    ) AOC_API_0_7;
//! The function oclirLink links in a series of modules of LLVM IR with the
// LLVM-IR that exists in the oclElf. The number of modules available is
// specified as the num argument. For each module, the size of the module
// in bytes is specified in the corresponding location in the size argument.
bool AOC_INTERFACE 
oclirLink(
    oclCompiler* cl,
    const oclElf* bin, 
    size_t** size, 
    const void **modules,
    size_t num, 
    oclLogFunction error_log
    ) AOC_API_0_7;
#ifdef __cplusplus
}
#endif
#endif // _API_LLVM_H_
