//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _COMPILER_LOADER_H_
#define _COMPILER_LOADER_H_
#include "v0_7/clTypes.h"
#include "v0_7/api_defs.h"
//! OpenCL API interfaces
#include "v0_7/plugins/interfaces/ocl.h"

//! Structure that holds all of the function
// pointers for the OCL api.
typedef struct _ocl_funcs_rec_0_7 {
  size_t          struct_size; // struct size for version check.
  CompileSource   compileSource;
  CompileSourceToIR   compileSourceToIR;
  CompileIRToIL   compileIRToIL;
  CompileILToISA  compileILToISA;
  CompileBinary   compileBinary;
  GetCompilerLog  getLog;
  InsertSection   insertSec;
  ExtractSection  extractSec;
  InsertSymbol    insertSym;
  ExtractSymbol   extractSym;
} OCL_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// OCL_FUNCS always points to the most recent version.
typedef OCL_FUNCS_0_7 OCL_FUNCS;

//! HSA API interfaces
#include "v0_7/plugins/interfaces/hsail.h"

//! Structure that holds all of the function
// pointers for the OCLHSA api.
typedef struct _hsail_funcs_rec_0_7 {
  size_t           struct_size; // struct size for version check.
  hsaCompileSource compileSource;
  hsaCompileBinary compileBinary;
  hsaSetType       setHSAIL;
  hsaRetrieveType  retrieveHSAIL;
  hsaConvertType   convertHSAIL;
} HSAIL_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// HSAIL_FUNCS always points to the most recent version.
typedef HSAIL_FUNCS_0_7 HSAIL_FUNCS;


//! LLVM API interfaces
#include "v0_7/plugins/interfaces/llvm.h"

//! Structure that holds all of the function
// pointers for the OCLIR api.
typedef struct _llvm_funcs_rec_0_7 {
  size_t          struct_size; // struct size for version check.
  irSetType       setIR;
  irRetrieveType  retrieveIR;
  irLink          linkIR;
} LLVM_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// LLVM_FUNCS always points to the most recent version.
typedef LLVM_FUNCS_0_7 LLVM_FUNCS;

//! X86 API interfaces
#include "v0_7/plugins/interfaces/x86.h"

//! Structure that holds all of the function
// pointers for the OCL api.
typedef struct _x86_funcs_rec_0_7 {
  size_t struct_size; // struct size for version check.
} X86_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// OCL_FUNCS always points to the most recent version.
typedef X86_FUNCS_0_7 X86_FUNCS;

//! AMDIL API interfaces
#include "v0_7/plugins/interfaces/amdil.h"

//! Structure that holds all of the function
// pointers for the OCLIL api.
typedef struct _amdil_funcs_rec_0_7 {
  size_t          struct_size; // struct size for version check.
  ilCompileSource compileSource;
  ilCompileBinary compileBinary;
  ilSetType       setType;
  ilRetrieveType  retrieveType;
  ilConvertType   convertType;
} AMDIL_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// AMDIL_FUNCS always points to the most recent version.
typedef AMDIL_FUNCS_0_7 AMDIL_FUNCS;

//! ISA API interfaces
#include "v0_7/plugins/interfaces/gpuisa.h"

//! Structure that holds all of the function
// pointers for the OCLISA api.
typedef struct _oclisa_funcs_rec_0_7 {
  size_t            struct_size; // struct size for version check.
  isaDisassemble    disassemble;
  isaGetBinaryBlob  retrieveISA;
} ISA_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// ISA_FUNCS always points to the most recent version.
typedef ISA_FUNCS_0_7 ISA_FUNCS;

//! RT API interfaces
#include "v0_7/plugins/interfaces/rt.h"

//! Structure that holds all of the function
// pointers for the OCLRT api.
typedef struct _rt_funcs_rec_0_7 {
  size_t struct_size; // struct size for version check.
  oclrtGetInfo getInfo;
} RT_FUNCS_0_7;


//! Typedef of the current version of the structure so that
// RT_FUNCS always points to the most recent version.
typedef RT_FUNCS_0_7 RT_FUNCS;

//! CompilerLoader struct
//! Structure that holds all of the function required for the
// compiler loader to handle the OCL Compiler Lib API corrGectly.
typedef struct _compiler_loader_rec_0_7 {
  size_t      struct_size; // Struct size for version check.
  bool        isBuiltinCL;
  OCL_FUNCS   ocl;
  HSAIL_FUNCS oclhsa;
  LLVM_FUNCS  oclir;
  AMDIL_FUNCS oclil;
  ISA_FUNCS   oclisa;
  X86_FUNCS   oclx86;
  RT_FUNCS    oclrt;
} CompilerLoader_0_7;

//! Typedef of the current compiler loader so that the CompilerLoader
// type always points to the most recent version.
typedef CompilerLoader_0_7 CompilerLoader;

//! The function SetupCLLoader() creats a CompilerLoader object 
// and correctl initializes it. The caller of this function must
// also call TeardownCLLoader() so any dynamically allocated 
// memory is released.
CompilerLoader* AOC_INTERFACE  SetupCLLoader();

//! The function TeardownCLLoader() uninitializes any memory that
// was allocated by the setup function and if needed unloads
// the DLL.
void AOC_INTERFACE  TeardownCLLoader(CompilerLoader **loader);

#endif // _COMPILER_LOADER_H_
