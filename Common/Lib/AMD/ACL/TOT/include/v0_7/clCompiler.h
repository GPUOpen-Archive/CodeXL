//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _CL_API_COMPILER_H_
#define _CL_API_COMPILER_H_

#ifdef __cplusplus
extern "C" {
#endif
// Header file that holds all of the enums and data types that are defined
// for the OpenCL compiler lib.
#include "v0_7/clTypes.h"

//! The oclCompilerInit() function initializes the compiler and returns
// a handle to the compiler that is used in the rest of the API. 
oclCompiler* AOC_INTERFACE 
oclCompilerInit() AOC_API_0_7;

//! The oclCompilerFini() function takes a compiler as an input and
// de-allocates all memory that was allocated for the compiler
bool AOC_INTERFACE 
oclCompilerFini(
    oclCompiler* CL
    ) AOC_API_0_7;

//! Returns the version of the compiler passed in as an argument. The versions
// number is based on the accumulated size of all the struct_size arguments 
// of all structs used in the compiler. 
oclCLVersion AOC_INTERFACE 
oclCompilerVersion(
    oclCompiler* CL
    ) AOC_API_0_7;

//! Returns the version of the compiler passed in as an argument. 
// The versions  number is based on the accumulated size of all the struct_size
// arguments of all structs used in the compiler. This is equivalent to the 
// serialized size of the compiler.
uint32_t AOC_INTERFACE  
oclVersionSize(
    oclCLVersion num
    ) AOC_API_0_7;

//!--------------------------------------------------------------------------!//
// Functions to query architectures, family, device and asics
//!--------------------------------------------------------------------------!//
//! Returns in the names argument, if non-NULL, a pointer to each of the arch 
// names that the compiler supports. If names is NULL and arch_size is 
// non-NULL, returns the number of arch entries that are required.
void AOC_INTERFACE 
  oclGetArchInfo(const char** names, size_t *arch_size) AOC_API_0_7;

//! Returns in the names argument, if non-NULL, a pointer to each family
// name that the compiler supports. If family_size is non-NULL,
// returns the number of family entries that are used.
// The arch argument should point to the name of the arch that 
// one wants to compile for.
void AOC_INTERFACE 
  oclGetFamilyInfo(const char* arch, const char** names, size_t *family_size) AOC_API_0_7;

//! Returns in the names argument, if non-NULL, a pointer to each device 
// name that the compiler supports. If device_size is non-NULL,
// returns the number of device entries that are used.
// The family argument should point to the name of the family that one
// wants to compile for.
void AOC_INTERFACE 
  oclGetDeviceInfo(const char* family, const char** names, size_t *device_size) AOC_API_0_7;

//! Returns in the names argument, if non-NULL, a pointer to each asic
// name that the compiler suppors for the chip. If asic_size is non-NULL,
// returns the number of asic entries that are used.
// The device argument should point to the name of the device that
// one wants to compile for.
void AOC_INTERFACE 
  oclGetAsicInfo(const char* chip, const char** names, size_t *asic_size) AOC_API_0_7;


//! Function that returns a correctly filled out oclTargetInfo structure based
// on the information passed into the kernel.
oclTargetInfo AOC_INTERFACE 
oclGetTargetInfo(const char* arch, 
                 const char* device, 
                 const char* chip, 
                 const char* asic) AOC_API_0_7;

//! Function that returns a string representation of the target architecture.
const char*  AOC_INTERFACE
    oclGetArchitecture(const oclTargetInfo &target) AOC_API_0_7;

//! Function that returns a string representation of the target family.
const char* AOC_INTERFACE 
    oclGetFamily(const oclTargetInfo &target) AOC_API_0_7;

//! Function that returns a string representation of the target chip.
const char* AOC_INTERFACE 
    oclGetChip(const oclTargetInfo &target) AOC_API_0_7;

//! Function that returns a string representation of the target asic.
const char* AOC_INTERFACE 
    oclGetAsic(const oclTargetInfo &target) AOC_API_0_7;

//!--------------------------------------------------------------------------!//
//           API for constructing/destructing Compiler Lib objects.
//!--------------------------------------------------------------------------!//
//! Construct an oclElf.
oclElf* AOC_INTERFACE 
    constructOclElf(size_t struct_version) AOC_API_0_7;

//! Destruct and oclElf.
bool AOC_INTERFACE 
    destructOclElf(oclElf* bin) AOC_API_0_7;

//! Read an oclElf from a file and upgrade the contained BIF to the most recent version.
oclElf* AOC_INTERFACE 
    readOclElfFromFile(const char* str) AOC_API_0_7;

//! Read an oclElf from memory and upgrade the contained BIF to the most recent version.
oclElf* AOC_INTERFACE 
    readOclElfFromMem(char* mem, size_t size) AOC_API_0_7;

//! Write an oclElf BIF to the file specifid by str.
bool AOC_INTERFACE 
    writeOclElfToFile(const char* str, oclElf *bin) AOC_API_0_7;

//! Write an oclElf BIF to the memory allocated in the mem pointer of the size.
bool AOC_INTERFACE 
    writeOclElfToMem(char** mem, size_t *size, oclElf *bin) AOC_API_0_7;

//! create an oclElf from another oclElf in the format specified by the version field
// Warning: Create an elf that is not the latest version can cause information loss.
oclElf* AOC_INTERFACE 
    createElfFromElf(const oclElf *elf, bifVersion version) AOC_API_0_7;

//! Get the current bif version from the oclElf.
bifVersion AOC_INTERFACE 
    getBIFVersion(const oclElf *elf) AOC_API_0_7;

//!--------------------------------------------------------------------------!//
//                     Compiler Lib OpenCL API 
//!--------------------------------------------------------------------------!//
#include "ocl.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib LLVM API 
//!--------------------------------------------------------------------------!//
#include "llvm.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib X86 API 
//!--------------------------------------------------------------------------!//
#include "x86.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib HSAIL API 
//!--------------------------------------------------------------------------!//
#include "hsail.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib AMDIL API 
//!--------------------------------------------------------------------------!//
#include "amdil.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib ISA API 
//!--------------------------------------------------------------------------!//
#include "gpuisa.h"

//!--------------------------------------------------------------------------!//
//                     Compiler Lib RT API 
//!--------------------------------------------------------------------------!//
#include "rt.h"
#ifdef __cplusplus
}
#endif
#endif // _CL_API_COMPILER_H_
