//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _INTERFACE_RT_H_
#define _INTERFACE_RT_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
// Function used to query the elf for information that is needed by the 
// runtime. The function will return true if there is no error and the 
// results of the query in the ptr/size fields. If ptr is NULL, only the
// size of space that is required is returned, if size is NULL, it is an
// error. If the return type is a pointer, then the size field contains
// the number of elements of the type.
typedef bool
( AOC_INTERFACE *oclrtGetInfo)(
    oclCompiler *cl, 
    const oclElf *bin, 
    unsigned query, 
    const char *kernel, 
    void *ptr, 
    size_t *size) AOC_API_0_7;
#endif // _INTERFACE_RT_H_
