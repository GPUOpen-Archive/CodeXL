//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _ACL_0_8_H_
#define _ACL_0_8_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "aclTypes.h"

//!--------------------------------------------------------------------------!//
// Functions that deal with aclCompiler objects.
//!--------------------------------------------------------------------------!//
aclCompiler* ACL_API_ENTRY
aclCompilerInit(aclCompilerOptions *opts, acl_error *error_code) ACL_API_0_8;
acl_error ACL_API_ENTRY
  aclCompilerFini(aclCompiler *cl) ACL_API_0_8;
aclCLVersion ACL_API_ENTRY
  aclCompilerVersion(aclCompiler *cl, acl_error *error_code) ACL_API_0_8;
uint32_t ACL_API_ENTRY
  aclVersionSize(aclCLVersion num, acl_error *error_code) ACL_API_0_8;
const char* ACL_API_ENTRY
  aclGetErrorString(acl_error error_code) ACL_API_0_8;

//!--------------------------------------------------------------------------!//
// Functions that deal with target specific information.
//!--------------------------------------------------------------------------!//
//! Returns in the names argument, if non-NULL, a pointer to each of the arch
// names that the compiler supports. If names is NULL and arch_size is
// non-NULL, returns the number of arch entries that are required.
acl_error ACL_API_ENTRY
  aclGetArchInfo(const char** arch_names,
      size_t *arch_size) ACL_API_0_8;

//! Returns in the arch argument, if non-NULL, a pointer to each device
// name that the compiler supports. If device_size is non-NULL,
// returns the number of device entries that are used.
acl_error ACL_API_ENTRY
  aclGetDeviceInfo(const char* arch,
      const char **names,
      size_t *device_size) ACL_API_0_8;

//! Function that returns a correctly filled out aclTargetInfo structure based
// on the information passed into the kernel.
aclTargetInfo ACL_API_ENTRY
aclGetTargetInfo(const char *arch,
                 const char *device,
                 acl_error *error_code) ACL_API_0_8;

//! Function that returns a correctly filled out aclTargetInfo structure based
// on the information passed into the kernel.
aclTargetInfo ACL_API_ENTRY
aclGetTargetInfoFromChipID(const char *arch,
                 const uint32_t chip_id,
                 acl_error *error_code) ACL_API_0_8;

//! Function that returns a string representation of the target architecture.
const char* ACL_API_ENTRY
  aclGetArchitecture(const aclTargetInfo &target) ACL_API_0_8;

//! Function that returns a string representation of the target chip options.
const uint64_t ACL_API_ENTRY
  aclGetChipOptions(const aclTargetInfo &target) ACL_API_0_8;

//! Function that returns a string representation of the target family.
const char* ACL_API_ENTRY
  aclGetFamily(const aclTargetInfo &target) ACL_API_0_8;

//! Function that returns a string representation of the target chip.
const char* ACL_API_ENTRY
  aclGetChip(const aclTargetInfo &target) ACL_API_0_8;

//!--------------------------------------------------------------------------!//
// Functions that deal with aclBinary objects.
//!--------------------------------------------------------------------------!//
aclBinary* ACL_API_ENTRY
  aclBinaryInit(
    size_t struct_version,
    const aclTargetInfo *target,
    const aclBinaryOptions *options,
    acl_error *error_code) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclBinaryFini(aclBinary *bin) ACL_API_0_8;

aclBinary* ACL_API_ENTRY
  aclReadFromFile(const char *str,
    acl_error *error_code) ACL_API_0_8;

aclBinary* ACL_API_ENTRY
  aclReadFromMem(const void *mem,
    size_t size, acl_error *error_code) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclWriteToFile(aclBinary *bin,
    const char *str) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclWriteToMem(aclBinary *bin,
    void **mem, size_t *size) ACL_API_0_8;

aclBinary* ACL_API_ENTRY
  aclCreateFromBinary(const aclBinary *binary,
    aclBIFVersion version) ACL_API_0_8;

aclBIFVersion ACL_API_ENTRY
  aclBinaryVersion(const aclBinary *binary) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclInsertSection(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclInsertSymbol(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id,
    const char *symbol) ACL_API_0_8;

const void* ACL_API_ENTRY
  aclExtractSection(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    acl_error *error_code) ACL_API_0_8;

const void* ACL_API_ENTRY
  aclExtractSymbol(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    const char *symbol,
    acl_error *error_code) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclRemoveSection(aclCompiler *cl,
    aclBinary *binary,
    aclSections id) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclRemoveSymbol(aclCompiler *cl,
    aclBinary *binary,
    aclSections id,
    const char *symbol) ACL_API_0_8;

//!--------------------------------------------------------------------------!//
// Functions that deal with debug/metdata.
//!--------------------------------------------------------------------------!//
acl_error ACL_API_ENTRY
  aclQueryInfo(aclCompiler *cl,
    const aclBinary *binary,
    aclQueryType query,
    const char *kernel,
    void *data_ptr,
    size_t *ptr_size) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclDbgAddArgument(aclCompiler *cl,
    aclBinary *binary,
    const char* kernel,
    const char* name,
    bool byVal)
    ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclDbgRemoveArgument(aclCompiler *cl,
    aclBinary *binary,
    const char* kernel,
    const char* name)
    ACL_API_0_8;

//!--------------------------------------------------------------------------!//
// Functions that deal with various compilation phases.
//!--------------------------------------------------------------------------!//
acl_error ACL_API_ENTRY
  aclCompile(aclCompiler *cl,
    aclBinary *bin,
    const char *options,
    aclType from,
    aclType to,
    aclLogFunction compile_callback) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclLink(aclCompiler *cl,
    aclBinary *src_bin,
    unsigned int num_libs,
    aclBinary **libs,
    aclType link_mode,
    const char *options,
    aclLogFunction link_callback) ACL_API_0_8;

const char* ACL_API_ENTRY
  aclGetCompilerLog(aclCompiler *cl) ACL_API_0_8;

const void* ACL_API_ENTRY
  aclRetrieveType(aclCompiler *cl,
    const aclBinary *bin,
    const char *name,
    size_t *data_size,
    aclType type,
    acl_error *error_code) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclSetType(aclCompiler *cl,
    aclBinary *bin,
    const char *name,
    aclType type,
    const void *data,
    size_t size) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclConvertType(aclCompiler *cl,
    aclBinary *bin,
    const char *name,
    aclType type) ACL_API_0_8;

acl_error ACL_API_ENTRY
  aclDisassemble(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    aclLogFunction disasm_callback) ACL_API_0_8;

const void* ACL_API_ENTRY
  aclGetDeviceBinary(aclCompiler *cl,
    const aclBinary *bin,
    const char *kernel,
    size_t *size,
    acl_error *error_code) ACL_API_0_8;

//!--------------------------------------------------------------------------!//
// Debug functionality
//!--------------------------------------------------------------------------!//
void aclDumpBinary(const aclBinary *bin);


//!--------------------------------------------------------------------------!//
// Functions that deal with memory.
// Free memory allocated by aclWriteToMem
//!--------------------------------------------------------------------------!//
acl_error ACL_API_ENTRY
aclFreeMem(aclBinary *bin,
    void *mem);
#ifdef __cplusplus
}
#endif
#endif // _ACL_0_8_H_
