//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_RT_H_
#define _API_RT_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"
#ifdef __cplusplus
extern "C" {
#endif
// This a handle of amd::Sym_Handle.
typedef void* ElfSymHandle;
// This must be an exact copy of SymbolInfo as found in elf/elf.h
typedef struct {
  char*     sec_name;    //!   section name
  char*     sec_addr;    //!   section address
  uint64_t  sec_size;    //!   section size
  char*     sym_name;    //!   symbol name
  char*     address;     //!   address of corresponding to symbol data
  uint64_t  size;        //!   size of data corresponding to symbol
} ElfSymbolInfo;

ElfSymHandle elfNextSymbol(const oclElf *elf, ElfSymHandle symbol);
bool elfGetSymbolInfo(const oclElf *elf, 
    ElfSymHandle symbol, 
    ElfSymbolInfo *syminfo);// Get the device name from the target information.

// Functions for runtime API
// Function used to query the elf for information that is needed by the 
// runtime. The function will return true if there is no error and the 
// results of the query in the ptr/size fields. If ptr is NULL, only the
// size of space that is required is returned, if size is NULL, it is an
// error. If the return type is a pointer, then the size field contains
// the number of elements of the type.
bool  AOC_INTERFACE rtGetInfo(oclCompiler *cl, const oclElf *bin, unsigned query, 
    const char *kernel, void *ptr, size_t *size);
#ifdef __cplusplus
}
#endif
#endif // _API_RT_H_

