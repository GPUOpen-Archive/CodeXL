//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _LLVM_LOADER_H_
#define _LLVM_LOADER_H_
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS 1
#endif

//! Typedef for the PreLinkerPass optimizer hook.
typedef bool AOC_INTERFACE ( AOC_INTERFACE *addPreLinkerPass)(char *PM, unsigned OptLevel);

//! Typedef for the PreOptPass optimizer hook.
typedef bool AOC_INTERFACE ( AOC_INTERFACE *addPreOptPass)(char *PM, unsigned OptLevel);

//! Typedef for the Post Opt optimizer hook.
typedef bool AOC_INTERFACE ( AOC_INTERFACE *addPostOptPass)(char *PM, unsigned OptLevel);

//! Structure that holds a series of function pointers that
// are used as hooks into the optimizer to allow plugin based
// optimizations. 
// The PreLinkerPass, PreOptPass and PostOptPass are module
// level hooks and the rest of function level hooks.
typedef struct _llvm_loader_rec_0_7 {
  size_t struct_size;
/* API goes here when finalized. */
	addPreLinkerPass	          preLink; /// Module pass.
	addPreOptPass		            preOpt; /// Module pass.
	addPostOptPass	            postOpt; /// Module pass.
/* Any version specific fields go here. */
} LLVMLoader_0_7;

// An enumeration that always points to the most recent
// version.
typedef LLVMLoader_0_7 LLVMLoader;

#endif // _LLVM_LOADER_H_
