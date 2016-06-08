//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _CL_API_TYPES_H_
#define _CL_API_TYPES_H_
#include "v0_7/api_defs.h"
#include "aclTypes.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _acl_options* bifOptions; 
typedef struct _acl_binary*  bifBinary;
typedef struct _acl_md_arg_type_0_8 argType;

//! Enum that represents the versions of the compiler
typedef enum _ocl_cl_version_rec {
  CLLIB_VERSION_ERROR  =  0,
  CLLIB_VERSION_0_7    =  1,
  CLLIB_VERSION_0_8    =  2,
  CLLIB_VERSION_0_9    =  3,
  CLLIB_VERSION_1_0    =  4,
  CLLIB_VERSION_LAST   =  5
} oclCLVersion;


// Enumeration for the various bif versions
typedef enum _elf_version_rec {
  bifVersionError = 0, // Error
  bifVersion20   = 1, // Version 2.0 of the OpenCL BIF
  bifVersion21   = 2, // Version 2.1 of the OpenCL BIF
  bifVersion30   = 3, // Version 3.0 of the OpenCL BIF
  bifVersionLatest = bifVersion30, // Most recent version of the BIF
  bifVersionLast = 4
} bifVersion;

typedef enum _ocl_dev_type_rec { 
  oclError  =  0, // aclDevType of 0 is an error.
  oclX86    =  1, // Targeting a X86 CPU device, only family required.
  oclAMDIL  =  2, // Targeting an AMDIL GPU device, family/chip/asic required.
  oclHSAIL  =  3, // Targeting an HSAIL GPU device, family/chip/asic required.
  oclLast   =  4
} oclDevType;

typedef enum _ocl_family_offset_rec {
  oclX86_OFFSET   =  0,
  oclAMDIL_OFFSET =  8,
  oclHSAIL_OFFSET = 16
} oclFamilyOffset;

//! An enumeration that defines the types of HSAIL formats that are supported.
typedef enum _ocl_hsa_type_rec {
  oclhsaError   =  0, // oclhsaType of 0 is an error
  oclhsaText    =  1, 
  oclhsaBinary  =  2,
  oclhsaLast    =  3
} oclhsaType;

// Enumeration for the various platform types
typedef enum bifPlatformEnum {
  bifPlatformCAL = 0, // For BIF 2.0 backward compatibility
  bifPlatformCPU = 1, // For BIF 2.0 backward compatibility
  bifPlatformCompLib = 2,
  bifPlatformLast = 3
} bifPlatform;

// Enumeration for the various bif sections
typedef enum oclSectionsEnum {
  bifLLVMIR = 0,
  bifSOURCE = 1,
  bifILTEXT = 2, // For BIF 2.0 backward compatibility
  bifASTEXT = 3, // For BIF 2.0 backward compatibility
  bifCAL    = 4, // For BIF 2.0 backward compatibility
  bifDLL    = 5, // For BIF 2.0 backward compatibility
  bifSTRTAB = 6,
  bifSYMTAB = 7,
  bifRODATA = 8,
  bifSHSTRTAB = 9,
  bifNOTES    = 10,
  bifCOMMENT  = 11,
  bifILDEBUG  = 12, // For BIF 2.0 backward compatibility
  bifDEBUG_INFO = 13,
  bifDEBUG_ABBREV = 14,
  bifDEBUG_LINE   = 15,
  bifDEBUG_PUBNAMES = 16,
  bifDEBUG_PUBTYPES = 17,
  bifDEBUG_LOC      = 18,
  bifDEBUG_ARANGES  = 19,
  bifDEBUG_RANGES   = 20,
  bifDEBUG_MACINFO  = 21,
  bifDEBUG_STR      = 22,
  bifDEBUG_FRAME    = 23,
  bifJITBINARY      = 24, // For BIF 2.0 backward compatibility
  bifCODEGEN        = 25,
  bifTEXT           = 26,
  bifINTERNAL       = 27,
  bifSPIR           = 28,
  bifLAST           = 29
} oclSections;

//! An enumeration that defines the type of LLVM-IR that are modifiable.
typedef enum _ocl_ir_type_rec {
  oclirError    =  0, // oclirType of 0 is an error
  oclirPreLink  =  1, 
  oclirPreOpt   =  2, 
  oclirPostOpt  =  3,
  oclirLast     =  4
} oclirType;

//! An enumaraion that defines the type of AMDIL that are supported.
typedef enum _ocl_il_type_rec {
  oclilError  =  0, // oclilType of 0 is an error
  oclilText   =  1, 
  oclilBinary =  2,
  oclilLast   =  3
} oclilType;

//! An structure that holds information on the capabilities of the elf device.
typedef struct _elf_device_caps_rec_0_7 {
  size_t struct_size;
  uint32_t flags[FLAG_ARRAY_SIZE];
  uint32_t encryptCode;
} elfDevCaps_0_7;

// A typedef that always point to the most recent version
// of elfDevCaps.
typedef struct _elf_device_caps_rec_0_7 elfDevCaps;

// A typedef that always points to the most recent
// version of oclTargetInfo.
typedef struct _ocl_target_info_rec_0_7 oclTargetInfo;

// A typedef that always points to the most recent
// version of oclElf.
typedef struct _ocl_elf_rec_0_7 oclElf;

// A typedef that always points to the most recent version of the compiler 
// object.
typedef struct _ocl_compiler_rec_0_7  oclCompiler;

//! Callback for the log function function pointer that many
// API calls take to have the calling application receive 
// information on what errors occur.
typedef void ( *oclLogFunction)(const char* msg, size_t size);

//! Structure that holds information on the target that the source is
// being compiled for.
typedef struct _ocl_target_info_rec_0_7 {
  size_t      struct_size; // Struct size that helps with version control.
  oclDevType  device_id; // An identifier for the device type.
  uint32_t    family_id; // A identifier for the family of devices.
  // The following two fields are not required for the CPU, except HSAIL CPU.
  uint32_t    chip_id; // A unique identifier for the chip.
  uint32_t    asic_id; // A unique identifier for each specific asic in a family.
} oclTargetInfo_0_7;

//! Structure that holds the OpenCL binary information.
typedef struct _ocl_elf_rec_0_7 {
  size_t                struct_size; // Size of structure for version checking.
  oclTargetInfo         target; // Information about the target device.
  bifBinary*            bin; // Pointer to the bif.
  bifOptions*           options; // Pointer to bif options.
  elfDevCaps            caps; // Capabilities of this elf.
} oclElfHandle_0_7;
#include "v0_7/plugins/CompilerLoader.h"
#include "v0_7/plugins/SCLoader.h"
  //! Structure that holds the OpenCL compiler and various loaders.
  typedef struct _ocl_compiler_rec_0_7 {
    size_t struct_size; // Size of structure for version checking.
    CompilerLoader* compAPI; // Pointer to the compiler API.
    size_t numLLVMLoaders; // Number of LLVM loaders that were discovered.
    void** loaders; // Pointer to array of LLVM Loader APIs
    aclSCLoader* scAPI; // Pointer to the SC Loader API.
    void *llvm_shutdown; // Pointer to the llvm shutdown object.
    char* buildLog;
    unsigned    logSize;
  } oclCompilerHandle_0_7;
#ifdef __cplusplus
}
#endif

#endif // _CL_API_TYPES_H_
