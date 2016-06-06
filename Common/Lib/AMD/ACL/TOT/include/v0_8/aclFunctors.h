//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _ACL_FUNCTORS_0_8_H_
#define _ACL_FUNCTORS_0_8_H_

//! Callback for the log function function pointer that many
// API calls take to have the calling application receive
// information on what errors occur.
typedef void (*aclLogFunction_0_8)(const char *msg, size_t size);

typedef acl_error
(ACL_API_ENTRY *InsertSec_0_8)(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *InsertSym_0_8)(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id,
    const char *symbol) ACL_API_0_8;

typedef const void *
(ACL_API_ENTRY *ExtractSec_0_8)(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    acl_error *error_code) ACL_API_0_8;

typedef const void *
(ACL_API_ENTRY *ExtractSym_0_8)(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    const char *symbol,
    acl_error *error_code) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *RemoveSec_0_8)(aclCompiler *cl,
    aclBinary *binary,
    aclSections id) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *RemoveSym_0_8)(aclCompiler *cl,
    aclBinary *binary,
    aclSections id,
    const char *symbol) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *QueryInfo_0_8)(aclCompiler *cl,
    const aclBinary *binary,
    aclQueryType query,
    const char *kernel,
    void *data_ptr,
    size_t *ptr_size) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *AddDbgArg_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    const char *name,
    bool byVal) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *RemoveDbgArg_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    const char *name) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *Compile_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *options,
    aclType from,
    aclType to,
    aclLogFunction_0_8 compile_callback) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *Link_0_8)(aclCompiler *cl,
    aclBinary *src_bin,
    unsigned int num_libs,
    aclBinary **libs,
    aclType link_mode,
    const char *options,
    aclLogFunction_0_8 link_callback) ACL_API_0_8;

typedef const char *
(ACL_API_ENTRY *CompLog_0_8)(aclCompiler *cl) ACL_API_0_8;

typedef const void *
(ACL_API_ENTRY *RetrieveType_0_8)(aclCompiler *cl,
    const aclBinary *bin,
    const char *name,
    size_t *data_size,
    aclType type,
    acl_error *error_code) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *SetType_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *name,
    aclType type,
    const void *data,
    size_t size) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *ConvertType_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *name,
    aclType type) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *Disassemble_0_8)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    aclLogFunction_0_8 disasm_callback) ACL_API_0_8;

typedef const void *
(ACL_API_ENTRY *GetDevBinary_0_8)(aclCompiler *cl,
    const aclBinary *bin,
    const char *kernel,
    size_t *size,
    acl_error *error_code) ACL_API_0_8;

typedef aclLoaderData *
(ACL_API_ENTRY *LoaderInit_0_8)(aclCompiler *cl,
    aclBinary *bin,
    aclLogFunction_0_8 callback,
    acl_error *error);

typedef acl_error
(ACL_API_ENTRY *LoaderFini_0_8)(aclLoaderData *data);

typedef aclModule *
(ACL_API_ENTRY *FEToIR_0_8)(aclLoaderData *ald,
    const char *source,
    size_t data_size,
    aclContext *ctx,
    acl_error *error) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *SourceToISA_0_8)(aclLoaderData *ald,
    const char *source,
    size_t data_size) ACL_API_0_8;

typedef aclModule *
(ACL_API_ENTRY *IRPhase_0_8)(aclLoaderData *data,
    aclModule *ir,
    aclContext *ctx,
    acl_error *error) ACL_API_0_8;

typedef aclModule *
(ACL_API_ENTRY *LinkPhase_0_8)(aclLoaderData *data,
    aclModule *ir,
    unsigned int num_libs,
    aclModule **libs,
    aclContext *ctx,
    acl_error *error) ACL_API_0_8;

typedef const void *
(ACL_API_ENTRY *CGPhase_0_8)(aclLoaderData *data,
    aclModule *ir,
    aclContext *ctx,
    acl_error *error) ACL_API_0_8;

typedef acl_error
(ACL_API_ENTRY *DisasmISA_0_8)(aclLoaderData *data,
    const char *kernel,
    const void *isa_code,
    size_t isa_size) ACL_API_0_8;

typedef void*
(*AllocFunc_0_8)(size_t size) ACL_API_0_8;

typedef void
(*FreeFunc_0_8)(void *ptr) ACL_API_0_8;

#endif // _ACL_FUNCTORS_0_8_H_
