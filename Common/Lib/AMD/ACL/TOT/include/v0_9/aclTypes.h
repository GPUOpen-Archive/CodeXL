//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _ACL_API_TYPES_0_9_H_
#define _ACL_API_TYPES_0_9_H_
#include <stdint.h>
#include <stddef.h>
/*!
 * @defgroup aclDefs09 aclDefines v0.9
 * @{
 */

/*!
 * \def ACL_API_ENTRY
 * Macro that is used on functions that are API calls.
 */
#ifndef ACL_API_ENTRY
#if defined(_WIN32) || defined(__CYGWIN__)
#define ACL_API_ENTRY __stdcall
#else
#define ACL_API_ENTRY
#endif
#endif

/*!
 * \def ACL_DEPRECATED
 * Macro that is used to mark deprecated functions.
 */
#ifndef ACL_DEPRECATED
#ifdef __GNU__C
#define ACL_DEPRECATED __attribute__((deprecated))
#elif defined(_WIN32) || defined(__CYGWIN__)
#define ACL_DEPRECATED __declspec(deprecated)
#else
#define ACL_DEPRECATED
#endif
#endif

/*!
 * \def ACL_API_0_9
 * Macro that is attached to all API v0.9 functions.
 */
#ifndef ACL_API_0_9
#define ACL_API_0_9
#endif

/*!
 * \def ACL_API_0_8
 * Macro that is attached to all API v0.8 functions.
 */
#ifndef ACL_API_0_8
#define ACL_API_0_8
#endif

/*!
 * \def ACL_API_0_7
 * Macro that is attached to all API v0.7 functions.
 */
#ifndef AOC_API_0_7
#define AOC_API_0_7 ACL_DEPRECATED
#endif

/*!
 * \def BIF_API_2_0
 * Macro that is attached to all BIF 2.0 API's.
 */
#ifndef BIF_API_2_0
#define BIF_API_2_0
#endif

/*!
 * \def BIF_API_2_1
 * Macro that is attached to all BIF 2.1 API's.
 */
#ifndef BIF_API_2_1
#define BIF_API_2_1
#endif

/*!
 * \def BIF_API_3_0
 * Macro that is attached to all BIF 3.0 API's.
 */
#ifndef BIF_API_3_0
#define BIF_API_3_0
#endif

/*!
 * \def BIF_API_3_1
 * Macro that is attached to all BIF 3.1 API's.
 */
#ifndef BIF_API_3_1
#define BIF_API_3_1
#endif
/** @} */
#
/** @defgroup aclTypes09 aclTypes v0.9
 * @{
 * Typedefs that always point to the most recent versions of the objects.
 */
typedef struct _acl_md_arg_type_0_9       aclArgData;
typedef struct _acl_md_printf_fmt_0_9     aclPrintfFmt;
typedef struct _acl_metadata_0_9          aclMetadata;
typedef struct _acl_device_caps_rec_0_9   aclDevCaps;
typedef struct _acl_target_info_rec_0_9   aclTargetInfo;
typedef struct _acl_bif_rec_0_9           aclBinary;
typedef struct _acl_binary_opts_rec_0_9   aclBinaryOptions;
typedef struct _acl_compiler_rec_0_9      aclCompiler;
typedef struct _acl_compiler_opts_rec_0_9 aclCompilerOptions;
/*! Opaque pointer to amd::Options. */
typedef struct _acl_options_0_9*          aclOptions;
/*! Opaque pointer to bifbase. */
typedef struct _acl_binary_0_9*           aclBIF;
typedef struct _acl_common_loader_rec_0_9 aclCommonLoader;
typedef struct _acl_cl_loader_rec_0_9     aclCLLoader;
typedef struct _acl_sc_loader_rec_0_9     aclSCLoader;
typedef struct _acl_fe_loader_rec_0_9     aclFELoader;
typedef struct _acl_link_loader_rec_0_9   aclLinkLoader;
typedef struct _acl_opt_loader_rec_0_9    aclOptLoader;
typedef struct _acl_cg_loader_rec_0_9     aclCGLoader;
typedef struct _acl_be_loader_rec_0_9     aclBELoader;
/*! Opaque pointer to llvm::Module. */
typedef struct _acl_llvm_module_0_9*      aclModule;
/*! Opaque pointer to llvm::Context. */
typedef struct _acl_llvm_context_0_9*     aclContext;
/*! Opaque pointer to loader data. */
typedef struct _acl_loader_data_0_9*      aclLoaderData;
typedef struct _acl_memory_obj_0_9        aclMemoryObj;
typedef struct _acl_key_map_0_9           aclKeyMap;
/** @} */

/*! @defgroup aclEnums09 aclEnums v0.9
 * @{
 */
/*! \enum _acl_error_enum_0_9
 * Enum that represents all of the possible error codes that
 * are returned by the compiler library.
 */
typedef enum _acl_error_enum_0_9 {
  /*! Success error code. */
  ACL_SUCCESS         = 0,
  /*! Generic error code. */
  ACL_ERROR           = 1,
  /*! Error code for invalid argument. */
  ACL_INVALID_ARG     = 2,
  /*! Error code when memory operation fails. */
  ACL_OUT_OF_MEM      = 3,
  /*! Error code for a system error. */
  ACL_SYS_ERROR       = 4,
  /*! Error code for an unsupported item. */
  ACL_UNSUPPORTED     = 5,
  /*! Error code for error in ELF library. */
  ACL_ELF_ERROR       = 6,
  /*! Error code for specifying an invalid file. */
  ACL_INVALID_FILE    = 7,
  /*! Error code for specifying an invalid compiler. */
  ACL_INVALID_COMPILER= 8,
  /*! Error code for specifying an invalid target. */
  ACL_INVALID_TARGET  = 9,
  /*! Error code for specifying an invalid binary. */
  ACL_INVALID_BINARY  = 10,
  /*! Error code for specifying an invalid option. */
  ACL_INVALID_OPTION  = 11,
  /*! Error code for specifying an invalid type. */
  ACL_INVALID_TYPE    = 12,
  /*! Error code for speecifying an invalid section. */
  ACL_INVALID_SECTION = 13,
  /*! Error code for specifying an invalid symbol. */
  ACL_INVALID_SYMBOL  = 14,
  /*! Error code for specifying an invalid query. */
  ACL_INVALID_QUERY   = 15,
  /*! Error code that occurs when there is a frontend problem. */
  ACL_FRONTEND_FAILURE= 16,
  /*! Error code that occurs when bitcode is invalid. */
  ACL_INVALID_BITCODE = 17,
  /*! Error code that occurs when linking fails. */
  ACL_LINKER_ERROR    = 18,
  /*! Error code that occurs when optimizer fails. */
  ACL_OPTIMIZER_ERROR = 19,
  /*! Error code caused by the code generator failing. */
  ACL_CODEGEN_ERROR   = 20,
  /*! Error code that occurs when ISA generation fails. */
  ACL_ISAGEN_ERROR    = 21,
  /*! Error code for invalid source files. */
  ACL_INVALID_SOURCE  = 22,
  /*! Error code that occurs when library creation fails. */
  ACL_LIBRARY_ERROR   = 23,
  /*! Error code for loading an invalid SPIR binary. */
  ACL_INVALID_SPIR    = 24,
  /*! Error code for when the lightweight verifier fails. */
  ACL_LWVERIFY_FAIL   = 25,
  /*! Error code for when the heavyweight verifier fails. */
  ACL_HWVERIFY_FAIL   = 26,
  /*! Error code for when there is an invalid label. */
  ACL_INVALID_LABEL   = 27,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_LAST_ERROR      = 28
} acl_error_0_9;

/*! \enum aclDevType_0_9
 * An enumeration that defines the possible valid device types that
 * can be compiled for.
 */
typedef enum _acl_dev_type_enum_0_9 {
  /*! aclDevType of 0 is an error.*/
  aclError   =  0,
  /*! Targeting a 32bit X86 CPU device.*/
  aclX86     =  1,
  /*! Targeting an AMDIL GPU device.*/
  aclAMDIL   =  2,
  /*! Targeting an HSAIL GPU device.*/
  aclHSAIL   =  3,
  /*! Targeting a 64bit X86 CPU device.*/
  aclX64     =  4,
  /*! Targeting a 64bit HSAIL GPU device.*/
  aclHSAIL64 =  5,
  /*! Targeting a 64bit AMDIL GPU device.*/
  aclAMDIL64 =  6,
  /*! Targeting a 32bit ARM CPU device.*/
  aclARM     =  7,
  /*! Targeting a 64bit ARM CPU device.*/
  aclA64     =  8,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  aclLast   =  9
} aclDevType_0_9;

/*! \enum aclCLVersion_0_9
 * Enum that represents the versions of the compiler */
typedef enum _acl_cl_version_enum_0_9 {
  /*! The error version number. */
  ACL_VERSION_ERROR  =  0,
  /*! The initial beta version number. */
  ACL_VERSION_0_7    =  1,
  /*! The second version of compiler library, used in HSAIL. */
  ACL_VERSION_0_8    =  2,
  /*! The first version of compiler library, used in OpenCL. */
  ACL_VERSION_0_8_1  =  3,
  /*! The current version of the compiler library. */
  ACL_VERSION_0_9    =  4,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_VERSION_LAST   =  5
} aclCLVersion_0_9;

/*! Enum of the various aclTypes that are supported that represent
 * either compilation stages or source and destination targets. */
typedef enum _acl_type_enum_0_9 {
  /*! The error type. */
  ACL_TYPE_ERROR          =  0,
  /*! Type that represents OpenCL C/C++ source. */
  ACL_TYPE_OPENCL         =  1,
  /*! Type that represents LLVM-IR Text. */
  ACL_TYPE_LLVMIR_TEXT    =  2,
  /*! Type that represents LLVM-IR Binary. */
  ACL_TYPE_LLVMIR_BINARY  =  3,
  /*! Type that represents SPIR Text. */
  ACL_TYPE_SPIR_TEXT      =  4,
  /*! Type that represents SPIR Binary. */
  ACL_TYPE_SPIR_BINARY    =  5,
  /*! Type that represents AMDIL Text. */
  ACL_TYPE_AMDIL_TEXT     =  6,
  /*! Type that represents AMDIL Binary. */
  ACL_TYPE_AMDIL_BINARY   =  7,
  /*! Type that represents HSAIL Text. */
  ACL_TYPE_HSAIL_TEXT     =  8,
  /*! Type that represents HSAIL Binary(BRIG). */
  ACL_TYPE_HSAIL_BINARY   =  9,
  /*! Type that represents X86 Text. */
  ACL_TYPE_X86_TEXT       = 10,
  /*! Type that represents X86 Binary. */
  ACL_TYPE_X86_BINARY     = 11,
  /*! Type that represents post code generation phase. */
  ACL_TYPE_CG             = 12,
  /*! Type that represents post isa generation phase. */
  ACL_TYPE_ISA            = 13,
  /*! Type that represents a header file. */
  ACL_TYPE_HEADER         = 14,
  /*! Type that represents ARM Text. */
  ACL_TYPE_ARM_TEXT       = 15,
  /*! Type that represents ARM Binary. */
  ACL_TYPE_ARM_BINARY     = 16,
  /*! Type that represents C++ AMP source. */
  ACL_TYPE_CPP_AMP        = 17,
  /*! Type that represents post frontend phase. */
  ACL_TYPE_POST_FE        = 18,
  /*! Type that represents post library linking phase. */
  ACL_TYPE_POST_LIB       = 19,
  /*! Type that represents post optimization phase. */
  ACL_TYPE_POST_OPT       = 20,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_TYPE_LAST           = 21
} aclType_0_9;

/*! \enum aclLoaderType_0_9
 * Enum of the various loader types that are supported.
 */
typedef enum _acl_loader_type_enum_0_9 {
  /*! Loader for the compiler library. */
  ACL_LOADER_COMPLIB  = 0,
  /*! Loader for the frontend. */
  ACL_LOADER_FRONTEND = 1,
  /*! Loader for the linker. */
  ACL_LOADER_LINKER   = 2,
  /*! Loader for the optimizer. */
  ACL_LOADER_OPTIMIZER= 3,
  /*! Loader for the codegen. */
  ACL_LOADER_CODEGEN  = 4,
  /*! Loader for the backends. */
  ACL_LOADER_BACKEND  = 5,
  /*! Loader for the shader compiler. */
  ACL_LOADER_SC       = 6,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_LOADER_LAST     = 7
} aclLoaderType_0_9;

/*! \enum aclBifVersion_0_9
 * Enumeration for the various acl versions
 */
typedef enum _bif_version_enum_0_9 {
  /*! Invalid bif version. */
  aclBIFVersionError = 0, // Error
  /*! CAL binary. */
  aclBIFVersionCAL  = 1,
  /*! BIF Version 2.0. */
  aclBIFVersion20   = 2,
  /*! BIF Version 2.1. */
  aclBIFVersion21   = 3,
  /*! BIF Version 3.0. */
  aclBIFVersion30   = 4,
  /*! BIF Version 3.1. */
  aclBIFVersion31   = 5,
  /*! Enum to the most recent version of the BIF. */
  aclBIFVersionLatest = aclBIFVersion31,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  aclBIFVersionLast = 6
} aclBIFVersion_0_9;

/*! \enum aclPlatform_0_9
 * Enumeration for the various platform types
 */
typedef enum _bif_platform_enum_0_9 {
  /*! BIF 2.X CAL platform. */
  aclPlatformCAL = 0,
  /*! BIF 2.X CPU platform. */
  aclPlatformCPU = 1,
  /*! Compiler library platform. */
  aclPlatformCompLib = 2,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  aclPlatformLast = 3
} aclPlatform_0_9;

/*! \enum aclSections_0_9
 * Enumeration for the various bif sections.
 */
typedef enum _bif_sections_enum_0_9 {
  /*! .llvmir */
  aclLLVMIR         = 0,
  /*! .source */
  aclSOURCE         = 1,
  /*! .amdil(BIF 2.X only) */
  aclILTEXT         = 2,
  /*! .astext(BIF 2.X only) */
  aclASTEXT         = 3,
  /*! .text(BIF 2.X only) */
  aclCAL            = 4,
  /*! .text(BIF 2.X only) */
  aclDLL            = 5,
  /*! .strtab */
  aclSTRTAB         = 6,
  /*! .symtab */
  aclSYMTAB         = 7,
  /*! .rodata */
  aclRODATA         = 8,
  /*! .shstrtab */
  aclSHSTRTAB       = 9,
  /*! .note */
  aclNOTES          = 10,
  /*! .comment */
  aclCOMMENT        = 11,
  /*! .debugil(BIF 2.X only) */
  aclILDEBUG        = 12,
  /*! .debug_info */
  aclDEBUG_INFO     = 13,
  /*! .debug_abbrev */
  aclDEBUG_ABBREV   = 14,
  /*! .debug_line */
  aclDEBUG_LINE     = 15,
  /*! .debug_pubnames */
  aclDEBUG_PUBNAMES = 16,
  /*! .debug_pubtypes */
  aclDEBUG_PUBTYPES = 17,
  /*! .debug_loc */
  aclDEBUG_LOC      = 18,
  /*! .debug_aranges */
  aclDEBUG_ARANGES  = 19,
  /*! .debug_ranges */
  aclDEBUG_RANGES   = 20,
  /*! .debug_macinfo */
  aclDEBUG_MACINFO  = 21,
  /*! .debug_str */
  aclDEBUG_STR      = 22,
  /*! .debug_frame */
  aclDEBUG_FRAME    = 23,
  /*! .text(BIF 2.X only) */
  aclJITBINARY      = 24,
  /*! .cg */
  aclCODEGEN        = 25,
  /*! .text */
  aclTEXT           = 26,
  /*! .internal */
  aclINTERNAL       = 27,
  /*! .spir */
  aclSPIR           = 28,
  /*! .header */
  aclHEADER         = 29,
  /*! .brig_code */
  aclBRIGcode       = 30,
  /*! .brig_directives */
  aclBRIGdirs       = 31,
  /*! .brig_operands */
  aclBRIGoprs       = 32,
  /*! .brig_strtab */
  aclBRIGstrs       = 33,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  aclLAST           = 34
} aclSections_0_9;

/*! \enum aclmemObj_0_9
 * Enumeration for the type of memory object that is supported.
 */
typedef enum _acl_mem_obj_type_0_9 {
  /*! A memory object that only contains malloc/free. */
  aclSimpleMemObj   = 0,
  /*! A memory object that implements a C++ allocator. */
  aclFullMemObj     = 1,
  /*! A memory object that includes callback support. */
  aclCallbackMemObj = 2,
} aclMemObj_0_9;

/*! \enum aclQueryType_0_9
 * An enumeration that defines what are valid queries for aclQueryInfo.
 */
typedef enum _acl_query_types_enum_0_9 {
  /*! Query the metadata blob from the binary. */
  ACL_QUERY_METADATA = 0,
  /*! Query the ISA binary from the binary. */
  ACL_QUERY_BINARY   = 1,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_QUERY_LAST     = 2
} aclQueryType_0_9;

/*! \enum aclBinaryClass
 * Enumerations for the various types of binary classes that can be created.
 */
typedef enum _bif_class_enum_0_9 {
  /*! Specify that the binary is an unknown elf. */
  ACL_CLASS_UNKNOWN = 0,
  /*! Specify that the binary is a 32bit elf. */
  ACL_CLASS_ELF32   = 1,
  /*! Specify that the binary is a 64bit elf. */
  ACL_CLASS_ELF64   = 2,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_CLASS_LAST    = 3
} aclBIFClass_0_9;

/*! \enum aclBinaryEndian
 * Enumeration for the various types of binary endian that can be specified.
 */
typedef enum _bif_endian_enum_0_9 {
  /*! Specify that the binary has an unknown endianness. */
  ACL_ENDIAN_NONE = 0,
  /*! Specify that the binary has is little endian. */
  ACL_ENDIAN_LSB  = 1,
  /*! Specify that the binary has is big endian. */
  ACL_ENDIAN_MSB  = 2,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_ENDIAN_LAST = 3
} aclBIFEndian_0_9;

/*! \enum aclArgType_0_8
 * Enumerations for the various argument types that are supported in OpenCL.
 */
typedef enum _acl_arg_type_enum_0_8 {
  /*! The error argument. */
  ARG_TYPE_ERROR     = 0,
  /*! Argument type that represents sampler_t. */
  ARG_TYPE_SAMPLER   = 1,
  /*! Argument types that represents image_*_t types. */
  ARG_TYPE_IMAGE     = 2,
  /*! Argument types that represents counter*_t types. */
  ARG_TYPE_COUNTER   = 3,
  /*! Argument types that represents all pass by value types. */
  ARG_TYPE_VALUE     = 4,
  /*! Argument types that represents all pointer types.*/
  ARG_TYPE_POINTER   = 5,
  /*! Argument type that represents all semaphore*_t types. */
  ARG_TYPE_SEMAPHORE = 6,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ARG_TYPE_LAST      = 7
} aclArgType_0_8;

/*! \enum aclArgDataType_0_8
 * Enumerations of the valid data types for pass by value and
 * pass by pointer kernel arguments.
 */
typedef enum _acl_data_type_enum_0_8 {
  /*! Invalid data type. */
  DATATYPE_ERROR   =  0,
  /*! boolean data type. */
  DATATYPE_i1      =  1,
  /*! signed char data type. */
  DATATYPE_i8      =  2,
  /*! signed short data type. */
  DATATYPE_i16     =  3,
  /*! signed int data type. */
  DATATYPE_i32     =  4,
  /*! signed long data type. */
  DATATYPE_i64     =  5,
  /*! unsigned char data type. */
  DATATYPE_u8      =  6,
  /*! unsigned short data type. */
  DATATYPE_u16     =  7,
  /*! unsigned int data type. */
  DATATYPE_u32     =  8,
  /*! unsigned long data type. */
  DATATYPE_u64     =  9,
  /*! half data type. */
  DATATYPE_f16     = 10,
  /*! float data type. */
  DATATYPE_f32     = 11,
  /*! double data type. */
  DATATYPE_f64     = 12,
  /*! fp80 data type. */
  DATATYPE_f80     = 13,
  /*! fp128 data type. */
  DATATYPE_f128    = 14,
  /*! aggregate struct data type. */
  DATATYPE_struct  = 15,
  /*! aggregate union data type. */
  DATATYPE_union   = 16,
  /*! event_t type. */
  DATATYPE_event   = 17,
  /*! OpenCL specific data types(image_*_t, sampler_t, counter*_t). */
  DATATYPE_opaque  = 18,
  /*! Unknonw but valid data type. */
  DATATYPE_unknown = 19,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  DATATYPE_LAST    = 20
} aclArgDataType_0_8;

/*! \enum aclMemoryType_0_8
 * Enumerations of the valid memory types for pass by pointer
 * kernel arguments.
 */
typedef enum _acl_memory_type_enum_0_8 {
  /*! Error */
  PTR_MT_ERROR        = 0,
  /*! global buffer */
  PTR_MT_GLOBAL       = 1,
  /*! SW emulated private memory */
  PTR_MT_SCRATCH_EMU  = 2,
  /*! SW emulated local memory */
  PTR_MT_LDS_EMU      = 3,
  /*! uniformed access vector memory */
  PTR_MT_UAV          = 4,
  /*! SW emulated constant memory */
  PTR_MT_CONSTANT_EMU = 5,
  /*! SW emulated region memory */
  PTR_MT_GDS_EMU      = 6,
  /*! HW local memory */
  PTR_MT_LDS          = 7,
  /*! HW private memory */
  PTR_MT_SCRATCH      = 8,
  /*! HW constant memory */
  PTR_MT_CONSTANT     = 9,
  /*! HW region memory */
  PTR_MT_GDS          = 10,
  /*! SI and later HW private memory */
  PTR_MT_UAV_SCRATCH  = 11,
  /*! SI and later HW constant memory */
  PTR_MT_UAV_CONSTANT = 12,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  PTR_MT_LAST         = 13
} aclMemoryType_0_8;

/*! \enum aclAccessType_0_8
 * Enumeration that specifies the various access types for a pointer/image.
 */
typedef enum _acl_access_type_enum_0_8 {
  /*! Access type is wrong. */
  ACCESS_TYPE_ERROR = 0,
  /*! read_only access type. */
  ACCESS_TYPE_RO    = 1,
  /*! write_only access type. */
  ACCESS_TYPE_WO    = 2,
  /*! read_write access type. */
  ACCESS_TYPE_RW    = 3,
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACCESS_TYPE_LAST  = 4
} aclAccessType_0_8;
/*! @} */
#
/** @addtogroup aclTypes09
 * @{
 */
typedef enum   _acl_error_enum_0_9         acl_error;
typedef enum   _acl_dev_type_enum_0_9      aclDevType;
typedef enum   _acl_cl_version_enum_0_9    aclCLVersion;
typedef enum   _acl_type_enum_0_9          aclType;
typedef enum   _acl_loader_type_enum_0_9   aclLoaderType;
typedef enum   _acl_mem_obj_type_0_9       aclMemObjType;
typedef enum   _acl_query_types_enum_0_9   aclQueryType;
typedef enum   _acl_arg_type_enum_0_8      aclArgType;
typedef enum   _acl_data_type_enum_0_8     aclArgDataType;
typedef enum   _acl_memory_type_enum_0_8   aclMemoryType;
typedef enum   _acl_access_type_enum_0_8   aclAccessType;
typedef enum   _bif_version_enum_0_9       aclBIFVersion;
typedef enum   _bif_platform_enum_0_9      aclPlatform;
typedef enum   _bif_sections_enum_0_9      aclSections;
typedef enum   _bif_endian_enum_0_9        aclBIFEndian;
typedef enum   _bif_class_enum_0_9         aclBIFClass;
/** @} */


/** @defgroup aclFunctors09 aclFunctors v0.9
 * @{
 */
/*!
 * Callback for the log function function pointer that many
 * API calls take to have the calling application receive
 * information on what errors occur.
 */
typedef void (*aclLogFunction_0_9)(const char *msg, size_t size);

/*!
 * \copydoc aclInsertSection
 */
typedef acl_error
(ACL_API_ENTRY *InsertSec_0_9)(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id) ACL_API_0_9;

/*!
 * \copydoc aclInsertSymbol
 */
typedef acl_error
(ACL_API_ENTRY *InsertSym_0_9)(aclCompiler *cl,
    aclBinary *binary,
    const void *data,
    size_t data_size,
    aclSections id,
    const char *symbol) ACL_API_0_9;

/*!
 * \copydoc aclInsertLabel
 */
typedef acl_error
(ACL_API_ENTRY *InsertLbl_0_9)(aclCompiler *cl,
      aclBinary *binary,
      aclSections id,
      const char *label,
      size_t offset) ACL_API_0_9;
/*!
 * \copydoc aclExtractSection
 */
typedef const void *
(ACL_API_ENTRY *ExtractSec_0_9)(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    acl_error *error_code) ACL_API_0_9;

/*!
 * \copydoc aclExtractSymbol
 */
typedef const void *
(ACL_API_ENTRY *ExtractSym_0_9)(aclCompiler *cl,
    const aclBinary *binary,
    size_t *size,
    aclSections id,
    const char *symbol,
    acl_error *error_code) ACL_API_0_9;
/*!
 * \copydoc aclExtractLabel
 */
typedef const void*
(ACL_API_ENTRY ExtractLbl_0_9)(aclCompiler *cl,
      const aclBinary *binary,
      size_t *size,
      aclSections id,
      const char *label,
      acl_error *error_code) ACL_API_0_9;
/*!
 * \copydoc aclRemoveSection
 */
typedef acl_error
(ACL_API_ENTRY *RemoveSec_0_9)(aclCompiler *cl,
    aclBinary *binary,
    aclSections id) ACL_API_0_9;

/*!
 * \copydoc aclRemoveSymbol
 */
typedef acl_error
(ACL_API_ENTRY *RemoveSym_0_9)(aclCompiler *cl,
    aclBinary *binary,
    aclSections id,
    const char *symbol) ACL_API_0_9;
/*!
 * \copydoc aclRemoveLabel
 */
typedef acl_error
(ACL_API_ENTRY *RemoveLbl_0_9)(aclCompiler *cl,
      aclBinary *binary,
      aclSections id,
      const char *label) ACL_API_0_9;
/*!
 * \copydoc aclQueryInfo
 */
typedef acl_error
(ACL_API_ENTRY *QueryInfo_0_9)(aclCompiler *cl,
    const aclBinary *binary,
    aclQueryType query,
    const char *kernel,
    void *data_ptr,
    size_t *ptr_size) ACL_API_0_9;

/*!
 * \copydoc aclDbgAddArgument
 */
typedef acl_error
(ACL_API_ENTRY *AddDbgArg_0_9)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    const char *name,
    bool byVal) ACL_API_0_9;

/*!
 * \copydoc aclDbgRemoveArgument
 */
typedef acl_error
(ACL_API_ENTRY *RemoveDbgArg_0_9)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    const char *name) ACL_API_0_9;

/*!
 * \copydoc aclCompile
 */
typedef acl_error
(ACL_API_ENTRY *Compile_0_9)(aclCompiler *cl,
    aclBinary *bin,
    const char *options,
    aclType from,
    aclType to,
    aclLogFunction_0_9 compile_callback) ACL_API_0_9;

/*!
 * \copydoc aclLink
 */
typedef acl_error
(ACL_API_ENTRY *Link_0_9)(aclCompiler *cl,
    aclBinary *src_bin,
    unsigned int num_libs,
    aclBinary **libs,
    aclType link_mode,
    const char *options,
    aclLogFunction_0_9 link_callback) ACL_API_0_9;

/*!
 * \copydoc aclGetCompilerLog
 */
typedef const char *
(ACL_API_ENTRY *CompLog_0_9)(aclCompiler *cl) ACL_API_0_9;

/*!
 * \copydoc aclDisassemble
 */
typedef acl_error
(ACL_API_ENTRY *Disassemble_0_9)(aclCompiler *cl,
    aclBinary *bin,
    const char *kernel,
    aclLogFunction_0_9 disasm_callback) ACL_API_0_9;

/*!
 * \brief Functor for initializing a loader object.
 *
 */
typedef aclLoaderData *
(ACL_API_ENTRY *LoaderInit_0_9)(aclCompiler *cl,
    aclBinary *bin,
    aclLogFunction_0_9 callback,
    acl_error *error);

/*!
 * \brief Functor for finalizing a loader object.
 */
typedef acl_error
(ACL_API_ENTRY *LoaderFini_0_9)(aclLoaderData *data);

/*!
 * \brief Functor for a frontend to output LLVMIR.
 */
typedef aclModule *
(ACL_API_ENTRY *FEToIR_0_9)(aclLoaderData *ald,
    const char *source,
    size_t data_size,
    aclContext *ctx,
    acl_error *error) ACL_API_0_9;

/*!
 * \brief Functor to compile from Source to ISA.
 */
typedef acl_error
(ACL_API_ENTRY *SourceToISA_0_9)(aclLoaderData *ald,
    const char *source,
    size_t data_size) ACL_API_0_9;

/*!
 * \brief Functor to run optimizations on LLVMIR.
 */
typedef aclModule *
(ACL_API_ENTRY *IRPhase_0_9)(aclLoaderData *data,
    aclModule *ir,
    aclContext *ctx,
    acl_error *error) ACL_API_0_9;

/*!
 * \brief Functor to run linking of aclModules.
 */
typedef aclModule *
(ACL_API_ENTRY *LinkPhase_0_9)(aclLoaderData *data,
    aclModule *ir,
    unsigned int num_libs,
    aclModule **libs,
    aclContext *ctx,
    acl_error *error) ACL_API_0_9;

/*!
 * \brief Functor that runs the code generator.
 */
typedef const void *
(ACL_API_ENTRY *CGPhase_0_9)(aclLoaderData *data,
    aclModule *ir,
    aclContext *ctx,
    acl_error *error) ACL_API_0_9;

/*!
 * \brief Functor that runs the disassembler.
 */
typedef acl_error
(ACL_API_ENTRY *DisasmISA_0_9)(aclLoaderData *data,
    const char *kernel,
    const void *isa_code,
    size_t isa_size) ACL_API_0_9;

/** @defgroup aclMemAPI09 aclMemoryAPI v0.9
 * @{
 */
/*!
 * \brief Function pointer for memory allocation used by the compiler library.
 */
typedef void*
(*AllocFunc_0_9)(size_t size) ACL_API_0_9;

/*!
 * \brief Function pointer for memory deallocation used by the compiler library.
 */
typedef void
(*FreeFunc_0_9)(void *ptr) ACL_API_0_9;

/*!
 * \brief Function pointer for memory reallocation used by the compiler library.
 */
typedef void*
(*ReallocFunc_0_9)(void *ptr, size_t size) ACL_API_0_9;

/*!
 * \brief Function pointer for memory allocation and clear used by
 * the compiler library.
 */
typedef void
(*CallocFunc_0_9)(size_t num, size_t size) ACL_API_0_9;

/*!
 * \brief Memory allocation with callback data supplied as first argument.
 */
typedef void*
(*CBAllocFunc_0_9)(void *user, size_t size) ACL_API_0_9;

/*!
 * \brief Memory deallocation with callback data supplied as first argument.
 */
typedef void
(*CBFreeFunc_0_9)(void *user, void *ptr) ACL_API_0_9;

/*!
 * \brief Memory reallocation with callback data supplied as first argument.
 */
typedef void*
(*CBReallocFunc_0_9)(void *user, void *ptr, size_t size) ACL_API_0_9;

/*!
 * \brief Memory allocate/clear with callback data supplied as first argument.
 */
typedef void
(*CBCallocFunc_0_9)(void *user, size_t num, size_t size) ACL_API_0_9;
/** @} */
/** @} */


/** @addtogroup aclTypes09
 * @{
 */
typedef aclLogFunction_0_9 aclLogFunction;
typedef InsertLbl_0_9      InsertLbl;
typedef RemoveLbl_0_9      RemoveLbl;
typedef ExtractLbl_0_9     ExtractLbl;
typedef InsertSec_0_9      InsertSec;
typedef RemoveSec_0_9      RemoveSec;
typedef ExtractSec_0_9     ExtractSec;
typedef InsertSym_0_9      InsertSym;
typedef RemoveSym_0_9      RemoveSym;
typedef ExtractSym_0_9     ExtractSym;
typedef QueryInfo_0_9      QueryInfo;
typedef Compile_0_9        Compile;
typedef Link_0_9           Link;
typedef AddDbgArg_0_9      AddDbgArg;
typedef RemoveDbgArg_0_9   RemoveDbgArg;
typedef CompLog_0_9        CompLog;
typedef Disassemble_0_9    Disassemble;
typedef LoaderInit_0_9     LoaderInit;
typedef LoaderFini_0_9     LoaderFini;
typedef FEToIR_0_9         FEToIR;
typedef SourceToISA_0_9    SourceToISA;
typedef IRPhase_0_9        IRPhase;
typedef LinkPhase_0_9      LinkPhase;
typedef CGPhase_0_9        CGPhase;
typedef DisasmISA_0_9      DisasmISA;
typedef AllocFunc_0_9      AllocFunc;
typedef FreeFunc_0_9       FreeFunc;
typedef ReallocFunc_0_9    ReallocFunc;
typedef CallocFunc_0_9     CallocFunc;
typedef CBAllocFunc_0_9    CBAllocFunc;
typedef CBFreeFunc_0_9     CBFreeFunc;
typedef CBReallocFunc_0_9  CBReallocFunc;
typedef CBCallocFunc_0_9   CBCallocFunc;
/** @} */

// Round up to the next 32bit integer.
#define RU32B(A) ((((A) + 31) & ~31) / 32)
/** \def ACL_STRUCT_HEADER
 * The header for every structure that holds common data for all structures.
 * This information is used to determine the differences between the structures.
 */
#define ACL_STRUCT_HEADER \
  size_t struct_size

/** @defgroup aclStructs09 Version 9 of the aclStructures
 * @{
 */
/*!
 * A structure that holds information on the various types of arguments
 * The format in memory of this structure is
 * -------------
 * | aclArgData |
 * -------------
 * |->argStr    |
 * -------------
 * |->typeStr   |
 * -------------
 */
typedef struct _acl_md_arg_type_0_9 {
  ACL_STRUCT_HEADER;
  size_t argNameSize;
  size_t typeStrSize;
  const char *argStr;
  const char *typeStr;
  union {
    struct {
      /*! Struct for sampler arguments. */
      unsigned ID;
      unsigned isKernelDefined;
      unsigned value;
    } sampler;
    struct {
      /*! Struct for image arguments. */
      unsigned resID;
      unsigned cbNum;
      unsigned cbOffset;
      aclAccessType type;
      bool is2D;
      bool is1D;
      bool isArray;
      bool isBuffer;
    } image;
    struct {
      /*! struct for atomic counter arguments. */
      unsigned is32bit;
      unsigned resID;
      unsigned cbNum;
      unsigned cbOffset;
    } counter;
    struct {
      /*! struct for semaphore arguments. */
      unsigned resID;
      unsigned cbNum;
      unsigned cbOffset;
    } sema;
    struct {
      /*! struct for pass by value arguments. */
      unsigned numElements;
      unsigned cbNum;
      unsigned cbOffset;
      aclArgDataType data;
    } value;
    struct {
      /*! struct for pass by pointer arguments. */
      unsigned numElements;
      unsigned cbNum;
      unsigned cbOffset;
      unsigned bufNum;
      unsigned align;
      aclArgDataType data;
      aclMemoryType memory;
      aclAccessType type;
      bool isVolatile;
      bool isRestrict;
      bool isPipe;
    } pointer;
  } arg;
  aclArgType type;
  bool isConst;
} aclArgData_0_9;

/*! A structure that holds information for printf
 * The format in memory of this structure is
 * --------------
 * | aclPrintfFmt|
 * --------------
 * |->argSizes   |
 * --------------
 * |->fmrStr     |
 * --------------
 */

typedef struct _acl_md_printf_fmt_0_9 {
  ACL_STRUCT_HEADER;
  unsigned ID;
  size_t numSizes;
  size_t fmtStrSize;
  uint32_t *argSizes;
  const char *fmtStr;
} aclPrintfFmt_0_9;

/*! Structure that holds information on the target that the source is
 * being compiled for.
 */
typedef struct _acl_target_info_rec_0_9 {
  ACL_STRUCT_HEADER;
  /*! An identifier for the architecture. */
  aclDevType  arch_id;
  /*! A identifier for the chip. */
  uint32_t    chip_id;
} aclTargetInfo_0_9;

/**
 * \def ACL_LOADER_COMMON
 * \brief The common header for all ACL loader objects.
 *
 * The ACL_LOADER_COMMON macro holds the header for the
 * ACL loader objects. The header includes:
 *   - Flag to determine if the function is a builtin.
 *   - Pointer to the handle of the DLL.
 *   - Functor for initializing the loader.
 *   - Functor for finializing the loader.
 */
#define ACL_LOADER_COMMON\
  ACL_STRUCT_HEADER; \
bool isBuiltin; \
const char *libName; \
void       *handle; \
LoaderInit  init; \
LoaderFini  fini;

/*!
 * Struct that maps to the common structure between all loaders.
 */
typedef struct _acl_common_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
} aclCommonLoader_0_9;

/*!
 * Struct that holds all the functors to load the current
 * version of the compiler library.
 */
typedef struct _acl_cl_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! Functor to compiler binaries. */
  Compile       compile;
  /*! Functor to link binaries. */
  Link          link;
  /*! Functor to retrieve the compilation log. */
  CompLog       getLog;
  /*! Functor to disassemble the binary. */
  Disassemble   disassemble;
  /*! Functor to insert data to a section in the binary. */
  InsertSec     insSec;
  /*! Functor to extra data from a section in the binary. */
  ExtractSec    extSec;
  /*! Functor to remove a section from the binary. */
  RemoveSec     remSec;
  /*! Functor to insert a symbol and data into a binary. */
  InsertSym     insSym;
  /*! Functor to extract a data from a symbol in the binary. */
  ExtractSym    extSym;
  /*! Functor to remove a symbol and its data from the binary. */
  RemoveSym     remSym;
  /*! Functor to insert a label, but not data, into the binary. */
  InsertLbl     insLbl;
  /*! Functor to extract data at a label in the binary. */
  ExtractLbl    extLbl;
  /*! Functor to remove a label, but not data, from the binary. */
  RemoveLbl     remLbl;
  /*! Functor to query data from the binary. */
  QueryInfo     getInfo;
  /*! Functor to add debug information to the binary(AMDIL GPU only). */
  AddDbgArg     addDbg;
  /*! Functor to remove debug information from the binary(AMDIL GPU only). */
  RemoveDbgArg  removeDbg;
} aclCLLoader_0_9;

/*!
 * Structure that holds the required functions
 * that sc exports for the SCDLL infrastructure.
 */
typedef struct _acl_sc_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! SC version. */
  uint32_t /*SC_UINT32*/ sc_interface_version;
  /*! void pointer to the SC interface functions. */
  void /**SC_EXPORT_FUNCTIONS**/ *scef;
  /*! Any version specific fields go here.. */
} aclSCLoader_0_9;

/*!
 */
typedef struct _acl_fe_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! Used for Source to aclModule containing LLVMIR */
  FEToIR      toIR;
  /*! Used to convert raw SPIR/LLVM-IR to aclModule */
  FEToIR      toModule;
  /*! Used for Source to ISA. */
  SourceToISA toISA;
} aclFELoader_0_9;

/*!
 */
typedef struct _acl_opt_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! Used for IR to IR transformation. */
  IRPhase   optimize;
} aclOptLoader_0_9;

/*!
 */
typedef struct _acl_link_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! Used for Linking in IR modules. */
  LinkPhase link;
  /*! Used for converting SPIR to LLVMIR. */
  IRPhase   toLLVMIR;
  /*! Used for converting LLVMIR to SPIR. */
  IRPhase   toSPIR;
} aclLinkLoader_0_9;

/*!
 */
typedef struct _acl_cg_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*! Used for converting from LLVMIR to target ASM. */
  CGPhase  codegen;
} aclCGLoader_0_9;

/*!
 */
typedef struct _acl_be_loader_rec_0_9 {
  /*! Common loader code for all loader structures. */
  ACL_LOADER_COMMON;
  /*!Used for converting from target source to target ISA. */
  SourceToISA finalize;
  /*!Used for converting from target text to target binary. */
  SourceToISA assemble;
  /*!Used for converting from target binary to target ISA. */
  DisasmISA disassemble;
} aclBELoader_0_9;

/*! \enum aclKeyMap_0_9
 * The structure that maps to the metadata blob header.
 */
#define ACL_RESOURCE_MAX 256
#define ACL_SAMPLER_MAX 16
#define ACL_MAX_NUM_CB_CONST_USAGES 16
typedef struct _acl_key_map_0_9 {
  ACL_STRUCT_HEADER;
  uint8_t  ACL_MAJOR_VERSION;
  uint8_t  ACL_MINOR_VERSION;
  uint8_t  ACL_REVISION;
  /*! offset from base of structure to where memory for the zero terminated name exists. */
  uint64_t ACL_DEVICE_NAME;
  uint32_t ACL_WORK_GROUP_SIZE[3];
  uint32_t ACL_WORK_REGION_SIZE[3];
  uint32_t ACL_NUM_ARGUMENTS;
  /*! offset from the base of structure to where the argument data exists. */
  uint64_t ACL_ARGUMENT_DATA;
  uint32_t ACL_BINARY_SIZE;
  uint8_t  ACL_CPU_KERNEL_BARRIER;
  uint8_t  ACL_CPU_PROGRAM_BARRIER;
  uint8_t  ACL_GPU_SHADER_TYPE;
  uint32_t ACL_GPU_TEX_SAMPLER_USAGE;
  uint32_t ACL_GPU_CONST_BUF_USAGE;
  uint32_t ACL_GPU_NUM_CB_OPT_BUFFERS;
  /*! offset from the base of structure to where the immediate constant buffer exists. */
  uint64_t ACL_GPU_IMM_CONSTANT_BUFFER;
  uint16_t ACL_GPU_GPR_USAGE;
  uint8_t  ACL_GPU_FLOAT_MODE;
  uint8_t  ACL_GPU_IEEE_MODE;
  uint32_t ACL_GPU_SCRATCH_SIZE;
  uint32_t ACL_GPU_SC_NUM_DEBUG_SECTIONS;
  /*! offset from the base of structure to where the debug sections exist. */
  uint64_t ACL_GPU_SC_DEBUG_SECTIONS;
  uint32_t ACL_GPU_NUM_GDS_BYTES;
  uint32_t ACL_GPU_THREAD_SIZE_X;
  uint32_t ACL_GPU_THREAD_SIZE_Y;
  uint32_t ACL_GPU_THREAD_SIZE_Z;
  uint32_t ACL_GPU_NUM_THREAD_PER_GROUP;
  uint32_t ACL_GPU_EGNI_SHARED_GPR_USER;
  uint32_t ACL_GPU_EGNI_SHARED_GPR_TOTAL;
  /*! Should these be split up into their component parts? */
  uint32_t ACL_GPU_EGNI_reqSQ_LDS_ALLOC;
  /*! Should these be split up into their component parts? */
  uint32_t ACL_GPU_EGNI_reqSQ_PGM_RESOURCES_2_LS;
  /*! Should these be split up into their component parts? */
  uint32_t ACL_GPU_EGNI_reqSQ_PGM_RESOURCE_LS;
  uint32_t ACL_GPU_EGNI_CACHED_UAV_USAGE;
  uint32_t ACL_GPU_EGNI_NUM_CS_INPUT_SEMANTICS;
  uint64_t ACL_GPU_EGNI_CS_INPUT_SEMANTICS_DATA;
  uint32_t ACL_GPU_EGNI_RETURN_BUF_DWORD;
  uint32_t ACL_GPU_EGNI_RETURN_BUF_SHORT;
  uint32_t ACL_GPU_EGNI_RETURN_BUF_BYTE;
  uint32_t ACL_GPU_EGNI_RAT_OP_USED;
  uint32_t ACL_GPU_EGNI_RAT_ATOMIC_OP_USED;
  uint8_t  ACL_GPU_EGNI_SET_BUFFER_FOR_NUM_GROUP;
  uint32_t ACL_GPU_EGNI_PGM_END_CF;
  uint32_t ACL_GPU_EGNI_PGM_END_ALU;
  uint32_t ACL_GPU_EGNI_PGM_END_FETCH;
  /*! Offset from base structure where the EG/NI second immediate constant buffer exists. */
  uint64_t ACL_GPU_EGNI_IMM_CONSTANT_BUFFER_2;
  uint8_t  ACL_GPU_SI_ORDERED_APPEND;
  uint32_t ACL_GPU_SI_NUM_USER_ELEMENTS;
  /*! Offset from base of structure where the user elements exist. */
  uint64_t ACL_GPU_SI_USER_ELEMENTS;
  uint32_t ACL_GPU_SI_NUM_EXT_USER_ELEMENTS;
  /*! Offset from base of structure where the user ext elements exist. */
  uint64_t ACL_GPU_SI_EXT_USER_ELEMENTS;
  uint32_t ACL_GPU_SI_PGM_RSRC2;
  uint32_t ACL_GPU_SI_SGPR_USAGE;
  uint32_t ACL_AMDIL_FUNC_ID;
  uint32_t ACL_AMDIL_DEFAULT_RES_ID_UAV;
  uint8_t  ACL_AMDIL_DEFAULT_RES_ID_SCRATCH;
  uint8_t  ACL_AMDIL_DEFAULT_RES_ID_LDS;
  uint8_t  ACL_AMDIL_DEFAULT_RES_ID_GDS;
  uint8_t  ACL_AMDIL_DEFAULT_RES_ID_CONSTANT;
  uint8_t  ACL_AMDIL_COMPILER_WRITE;
  uint8_t  ACL_AMDIL_DATA_SECTION;
  uint8_t  ACL_AMDIL_REQUIRED_WGS;
  uint8_t  ACL_AMDIL_REQUIRED_WRS;
  uint8_t  ACL_AMDIL_LIMIT_WGS;
  uint8_t  ACL_AMDIL_PACKED_REGS;
  uint8_t  ACL_AMDIL_64BIT_ABI;
  uint8_t  ACL_AMDIL_PRINTF;
  uint8_t  ACL_AMDIL_ARENA_UAV;
  uint8_t  ACL_AMDIL_LRP_MEM;
  uint8_t  ACL_AMDIL_INDEXED_TEMPS;
  uint32_t ACL_AMDIL_HW_LOCAL;
  uint32_t ACL_AMDIL_HW_REGION;
  uint32_t ACL_AMDIL_HW_PRIVATE;
  uint32_t ACL_AMDIL_SW_LOCAL;
  uint32_t ACL_AMDIL_SW_REGION;
  uint32_t ACL_AMDIL_SW_PRIVATE;
  uint32_t ACL_AMDIL_NUM_PRINTFS;
  uint32_t ACL_GPU_TEX_RESOURCE_USAGE[RU32B(ACL_RESOURCE_MAX)];
  uint32_t ACL_GPU_UAV_RESOURCE_USAGE[RU32B(ACL_RESOURCE_MAX)];
  uint32_t ACL_GPU_TEX_SAMPLER_MAP[ACL_SAMPLER_MAX][RU32B(ACL_RESOURCE_MAX)];
  uint32_t ACL_GPU_CB_OPT_BUFFERS[ACL_MAX_NUM_CB_CONST_USAGES];
  /*! Offset from base of structure where the printf data exists. */
  uint64_t ACL_AMDIL_PRINTF_DATA;
  /*! Offset from base of structure where the custom key data blob exist. */
  uint64_t ACL_CUSTOM_KEYS;
  /*! Offset from the base of the structure that points to the last byte in the structure of all memory. */
  uint64_t ACL_KEYS_LAST;
} aclKeyMap_0_9;
/*!
 * \enum aclKeyValueEnum_0_9
 * These enums will map to the offset of a binary structure. The structure will
 * consist of a header followed by dynamic data. The header will contain all
 * data that does not vary in size between functions. The variable data will
 * queries will store a 64bit value that is the offset from the base of the
 * data where the data exists.
 */
typedef enum _acl_key_value_enum_0_9 {
  /*! The size of the structure itself used for versioning. */
  ACL_KEY_VALUE_VERSION = offsetof(aclKeyMap_0_9, struct_size),
  /*! The major version of the compiler library this was generated for. */
  ACL_MAJOR_VERSION = offsetof(aclKeyMap_0_9, ACL_MAJOR_VERSION),
  /*! The minor version of the compiler library this was generated for. */
  ACL_MINOR_VERSION = offsetof(aclKeyMap_0_9, ACL_MINOR_VERSION),
  /*! The revision of the compiler library this was generated for. */
  ACL_REVISION = offsetof(aclKeyMap_0_9, ACL_REVISION),
  /*! Query the name of the device. */
  ACL_DEVICE_NAME
      = offsetof(aclKeyMap_0_9, ACL_DEVICE_NAME),
  /*! Query the max work group size for the kernel. */
  ACL_WORK_GROUP_SIZE
      = offsetof(aclKeyMap_0_9, ACL_WORK_GROUP_SIZE),
  /*! Query the max work region size for the kernel. */
  ACL_WORK_REGION_SIZE
      = offsetof(aclKeyMap_0_9, ACL_WORK_REGION_SIZE),
  /*! Query the number of arguments for the kernel. */
  ACL_NUM_ARGUMENTS
      = offsetof(aclKeyMap_0_9, ACL_NUM_ARGUMENTS),
  /*! Query the argument data array. */
  ACL_ARGUMENT_DATA
      = offsetof(aclKeyMap_0_9, ACL_ARGUMENT_DATA),
  /*! Retrieve the size of the hardware binary. */
  ACL_BINARY_SIZE
      = offsetof(aclKeyMap_0_9, ACL_BINARY_SIZE),
  /*! Returns 1 if the kernel has a barrier call. */
  ACL_CPU_KERNEL_BARRIER
      = offsetof(aclKeyMap_0_9, ACL_CPU_KERNEL_BARRIER),
  /*! Returns 1 if the program has a barrier call. */
  ACL_CPU_PROGRAM_BARRIER
      = offsetof(aclKeyMap_0_9, ACL_CPU_PROGRAM_BARRIER),

  /*! Query the type of shader we are executing on the GPU. */
  ACL_GPU_SHADER_TYPE
      = offsetof(aclKeyMap_0_9, ACL_GPU_SHADER_TYPE),
  /*! Array of bitmasks showing which hardware texture was used. */
  ACL_GPU_TEX_RESOURCE_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_TEX_RESOURCE_USAGE),
  /*! Array of bitmasks showing which hardware UAV was used. */
  ACL_GPU_UAV_RESOURCE_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_UAV_RESOURCE_USAGE),
  /*! Array of bitmask showing which texture is used by a shader. */
  ACL_GPU_TEX_SAMPLER_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_TEX_SAMPLER_USAGE),
  /*! Array of bitmasks showing which constant buffer is used. */
  ACL_GPU_CONST_BUF_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_CONST_BUF_USAGE),
  /*! Array of bitmasks showing mapping between texture and sampler. */
  ACL_GPU_TEX_SAMPLER_MAP
      = offsetof(aclKeyMap_0_9, ACL_GPU_TEX_SAMPLER_MAP),
  /*! Number of constant buffer usages which SC could optimize if known. */
  ACL_GPU_NUM_CB_OPT_BUFFERS
      = offsetof(aclKeyMap_0_9, ACL_GPU_NUM_CB_OPT_BUFFERS),
  /*! Array of constants buffer usages that SC could recompile. */
  ACL_GPU_CB_OPT_BUFFERS
      = offsetof(aclKeyMap_0_9, ACL_GPU_CB_OPT_BUFFERS),
  /*! Retrieve the immediate constant buffers. */
  ACL_GPU_IMM_CONSTANT_BUFFER
      = offsetof(aclKeyMap_0_9, ACL_GPU_IMM_CONSTANT_BUFFER),
  /*! Retrieve the number of vector registers used.(VGPR on SI and later) */
  ACL_GPU_GPR_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_GPR_USAGE),
  /*! Retrieve the float mode of the kernel. */
  ACL_GPU_FLOAT_MODE
      = offsetof(aclKeyMap_0_9, ACL_GPU_FLOAT_MODE),
  /*! Return 1 if IEEE mode is set. */
  ACL_GPU_IEEE_MODE
      = offsetof(aclKeyMap_0_9, ACL_GPU_IEEE_MODE),
  /*! Return the scratch size as determined by SC. */
  ACL_GPU_SCRATCH_SIZE
      = offsetof(aclKeyMap_0_9, ACL_GPU_SCRATCH_SIZE),
  /*! Return the number of debug sections supported by SC. */
  ACL_GPU_SC_NUM_DEBUG_SECTIONS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SC_NUM_DEBUG_SECTIONS),
  /*! Fill out an array of memory for the debug sections. */
  ACL_GPU_SC_DEBUG_SECTIONS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SC_DEBUG_SECTIONS),
  /*! Number of GDS bytes reported to hardware. */
  ACL_GPU_NUM_GDS_BYTES
      = offsetof(aclKeyMap_0_9, ACL_GPU_NUM_GDS_BYTES),
  /*! Thread group size for X dimension. */
  ACL_GPU_THREAD_SIZE_X
      = offsetof(aclKeyMap_0_9, ACL_GPU_THREAD_SIZE_X),
  /*! Thread group size for Y dimension. */
  ACL_GPU_THREAD_SIZE_Y
      = offsetof(aclKeyMap_0_9, ACL_GPU_THREAD_SIZE_Y),
  /*! Thread group size for Z dimension. */
  ACL_GPU_THREAD_SIZE_Z
      = offsetof(aclKeyMap_0_9, ACL_GPU_THREAD_SIZE_Z),
  /*! Return the flattened number of threads per group. */
  ACL_GPU_NUM_THREAD_PER_GROUP
      = offsetof(aclKeyMap_0_9, ACL_GPU_NUM_THREAD_PER_GROUP),

  // Items unique to R8XX and R9XX hardware.
  /*! Number of shared GPR's from user. */
  ACL_GPU_EGNI_SHARED_GPR_USER
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_SHARED_GPR_USER),
  /*! Number of shared GPR's from total. */
  ACL_GPU_EGNI_SHARED_GPR_TOTAL
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_SHARED_GPR_TOTAL),
  /*! Return the LDS ALLOC register setup. */
  ACL_GPU_EGNI_reqSQ_LDS_ALLOC
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_reqSQ_LDS_ALLOC),
  /*! Return the reqSQ_PGM_RESOURCES_2_LS data. */
  ACL_GPU_EGNI_reqSQ_PGM_RESOURCES_2_LS
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_reqSQ_PGM_RESOURCES_2_LS),
  /*! Return the reqSQ_PGM_RESOURCE_LS data. */
  ACL_GPU_EGNI_reqSQ_PGM_RESOURCE_LS
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_reqSQ_PGM_RESOURCE_LS),
  /*! Return a bitmask that contains the cached uav resources. */
  ACL_GPU_EGNI_CACHED_UAV_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_CACHED_UAV_USAGE),
  /*! Return number of CS input semantics. */
  ACL_GPU_EGNI_NUM_CS_INPUT_SEMANTICS
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_NUM_CS_INPUT_SEMANTICS),
  /*! Return an array of CS shader input semantic declarations. */
  ACL_GPU_EGNI_CS_INPUT_SEMANTICS_DATA
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_CS_INPUT_SEMANTICS_DATA),
  /*! Return the global buffer for dwords. */
  ACL_GPU_EGNI_RETURN_BUF_DWORD
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_RETURN_BUF_DWORD),
  /*! Return the global buffer for shorts. */
  ACL_GPU_EGNI_RETURN_BUF_SHORT
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_RETURN_BUF_SHORT),
  /*! Return the global buffer for bytes. */
  ACL_GPU_EGNI_RETURN_BUF_BYTE
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_RETURN_BUF_BYTE),
  /*! Return a bitmask that shows which rat op is used. */
  ACL_GPU_EGNI_RAT_OP_USED
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_RAT_OP_USED),
  /*! Return a bitmask that shows which rat atomic op is used. */
  ACL_GPU_EGNI_RAT_ATOMIC_OP_USED
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_RAT_ATOMIC_OP_USED),
  /*! Return 1 if buffer #147 needs to be setup. */
  ACL_GPU_EGNI_SET_BUFFER_FOR_NUM_GROUP
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_SET_BUFFER_FOR_NUM_GROUP),
  /*! Return offset to end of program control flow. */
  ACL_GPU_EGNI_PGM_END_CF
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_PGM_END_CF),
  /*! Return offset to end of program ALU. */
  ACL_GPU_EGNI_PGM_END_ALU
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_PGM_END_ALU),
  /*! Return offset to end of program fetch. */
  ACL_GPU_EGNI_PGM_END_FETCH
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_PGM_END_FETCH),
  /*! Retrieve the second immediate constant buffers on EG/NI. */
  ACL_GPU_EGNI_IMM_CONSTANT_BUFFER_2
      = offsetof(aclKeyMap_0_9, ACL_GPU_EGNI_IMM_CONSTANT_BUFFER_2),

  // Items unique to SI hardware.
  /*! Return 1 if ordered append is on. */
  ACL_GPU_SI_ORDERED_APPEND
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_ORDERED_APPEND),
  /*! Retrieve the number of user elements. */
  ACL_GPU_SI_NUM_USER_ELEMENTS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_NUM_USER_ELEMENTS),
  /*! Retrieve array of SI User Elements. */
  ACL_GPU_SI_USER_ELEMENTS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_USER_ELEMENTS),
  /*! Retrieve the number of extended user elements. */
  ACL_GPU_SI_NUM_EXT_USER_ELEMENTS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_NUM_EXT_USER_ELEMENTS),
  /*! Retrieve array of SI extended user elements. */
  ACL_GPU_SI_EXT_USER_ELEMENTS
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_EXT_USER_ELEMENTS),
  /*! CS Compute Resource. */
  ACL_GPU_SI_PGM_RSRC2
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_PGM_RSRC2),
  /*! Retrieve the number of scalar registers used. */
  ACL_GPU_SI_SGPR_USAGE
      = offsetof(aclKeyMap_0_9, ACL_GPU_SI_SGPR_USAGE),

  /*! Returns the function ID for the AMDIL function name */
  ACL_AMDIL_FUNC_ID
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_FUNC_ID),
  /*! Returns the default resource ID for UAV's. */
  ACL_AMDIL_DEFAULT_RES_ID_UAV
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DEFAULT_RES_ID_UAV),
  /*! Returns the default resource ID for Scratch buffer or private UAV. */
  ACL_AMDIL_DEFAULT_RES_ID_SCRATCH
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DEFAULT_RES_ID_SCRATCH),
  /*! Returns the default resource ID for LDS. */
  ACL_AMDIL_DEFAULT_RES_ID_LDS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DEFAULT_RES_ID_LDS),
  /*! Returns the default resource ID for GDS. */
  ACL_AMDIL_DEFAULT_RES_ID_GDS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DEFAULT_RES_ID_GDS),
  /*! Returns the default resource ID for constant buffers. */
  ACL_AMDIL_DEFAULT_RES_ID_CONSTANT
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DEFAULT_RES_ID_CONSTANT),
  /*! Returns 1 if compiler write work-around exists for the kernel. */
  ACL_AMDIL_COMPILER_WRITE
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_COMPILER_WRITE),
  /*! Returns 1 if the kernel requires data section to be attached. */
  ACL_AMDIL_DATA_SECTION
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_DATA_SECTION),
  /*! Returns 1 if the kernel requires a specific work group size. */
  ACL_AMDIL_REQUIRED_WGS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_REQUIRED_WGS),
  /*! Returns 1 if the kernel requires a specific work region size. */
  ACL_AMDIL_REQUIRED_WRS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_REQUIRED_WRS),
  /*! Returns 1 if the kernel requires limiting the work group size. */
  ACL_AMDIL_LIMIT_WGS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_LIMIT_WGS),
  /*! Returns 1 if the kernel uses packed registers in debug mode. */
  ACL_AMDIL_PACKED_REGS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_PACKED_REGS),
  /*! Returns 1 if the 64bit version of the ABI is being utilized. */
  ACL_AMDIL_64BIT_ABI
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_64BIT_ABI),
  /*! Returns 1 if printf exists in the kernel. */
  ACL_AMDIL_PRINTF
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_PRINTF),
  /*! Returns 1 if the arena UAV is used in the kernel(EG/NI only). */
  ACL_AMDIL_ARENA_UAV
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_ARENA_UAV),
  /*! Returns 1 if local), region or private memory is being used. */
  ACL_AMDIL_LRP_MEM
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_LRP_MEM),
  /*! Returns 1 if indexed temporaries are being used. */
  ACL_AMDIL_INDEXED_TEMPS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_INDEXED_TEMPS),
  /*! Returns in bytes the number of hardware local memory being used. */
  ACL_AMDIL_HW_LOCAL
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_HW_LOCAL),
  /*! Returns in bytes the number of hardware region memory being used. */
  ACL_AMDIL_HW_REGION
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_HW_REGION),
  /*! Returns in bytes the number of hardware private memory being used. */
  ACL_AMDIL_HW_PRIVATE
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_HW_PRIVATE),
  /*! Returns in bytes the number of software local memory being used. */
  ACL_AMDIL_SW_LOCAL
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_SW_LOCAL),
  /*! Returns in bytes the number of software region memory being used. */
  ACL_AMDIL_SW_REGION
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_SW_REGION),
  /*! Returns in bytes the number of software private memory being used. */
  ACL_AMDIL_SW_PRIVATE
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_SW_PRIVATE),
  /*! Return the number of printf structures. */
  ACL_AMDIL_NUM_PRINTFS
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_NUM_PRINTFS),
  /*! Return the array of printf data. */
  ACL_AMDIL_PRINTF_DATA
      = offsetof(aclKeyMap_0_9, ACL_AMDIL_PRINTF_DATA),


  /*! A series of custom keys unique to a device that the compiler library
   * does not understand. */
  ACL_CUSTOM_KEYS
      = offsetof(aclKeyMap_0_9, ACL_CUSTOM_KEYS),
  /*! Canary Value that guards against enum changes
   * @warning This value cannot be changed without updating the appropriate
   * tests and should NEVER be decreased.
   */
  ACL_KEYS_LAST = offsetof(aclKeyMap_0_9, ACL_KEYS_LAST)
} aclKeyValueEnum_0_9;


/*!
 */
typedef struct _acl_memory_object_0_9 {
  /*! Common structure fields for version checking. */
  ACL_STRUCT_HEADER;
  /*! Field that specifies what the memory object type is. */
  aclMemObjType type;
  /*! Union of various memory object types. */
  union {
    /*! Memory object to provide version 0.8 support. */
    struct {
      /*! Functor used for allocating memory. */
      AllocFunc alloc;
      /*! Functor used for freeing memory. */
      FreeFunc  dealloc;
    } Simple;
    /*! Memory object that supports all the C stdlib memory functions. */
    struct {
      /*! Functor used for allocating memory. */
      AllocFunc     alloc;
      /*! Functor used for freeing memory. */
      FreeFunc    dealloc;
      /*! Functor re-allocating allocated memory. */
      ReallocFunc realloc;
      /*! Functor that zero-allocates N * M memory block. */
      CallocFunc   calloc;
    } Full;
    /*! Memory object that supports calling back to the host app. */
    struct {
      /*! Functor used for allocating memory. */
      CBAllocFunc     alloc;
      /*! Functor used for freeing memory. */
      CBFreeFunc    dealloc;
      /*! Functor re-allocating allocated memory. */
      CBReallocFunc realloc;
      /*! Functor that zero-allocates N * M memory block. */
      CBCallocFunc   calloc;
      /*! Pointer to data that is handed back to user. */
      void*        userdata;
    } Callback;
  };
} aclMemoryObject_0_9;

/*!
 * Structure for the version 0.8.1 of the structure.
 * This versions addes in alloc/dealloc functions.
 */
typedef struct _acl_binary_opts_rec_0_9 {
  ACL_STRUCT_HEADER;
  /*! Specify the elf class of the binary object. */
  aclBIFClass_0_9 elfclass;
  /*! Specify the bitness of the binary object. */
  aclBIFEndian_0_9 bitness;
  /*! Memory interface for the binary object. */
  aclMemoryObject_0_9 memory;
} aclBinaryOptions_0_9;

/*!
 * Version of the aclBinary that uses the 0_8_1 version of the
 * aclBinaryOptions.
 */
typedef struct _acl_bif_rec_0_9 {
  ACL_STRUCT_HEADER;
  /*! Information about the target device. */
  aclTargetInfo  target;
  /*! Pointer to the acl. */
  aclBIF *       bin;
  /*! Pointer to acl options. */
  aclOptions *   options;
  /*! Pointer to the binary options. */
  aclBinaryOptions binOpts;
} aclBinary_0_9;

/*!
*/
typedef struct _acl_compiler_opts_rec_0_9 {
  /*! Common structure fields for version checking. */
  ACL_STRUCT_HEADER;
  /*! Pointer to the dll that loads the compiler library. */
  const char *clLib;
  /*! Pointer to the dll that holds the frontend. */
  const char *feLib;
  /*! Pointer to the dll that holds the optimizer. */
  const char *optLib;
  /*! Pointer to the dll that holds the linker. */
  const char *linkLib;
  /*! Pointer to the dll that holds the code generator. */
  const char *cgLib;
  /*! Pointer to the dll that holds the backend. */
  const char *beLib;
  /*! Pointer to the dll that holds the shader compiler. */
  const char *scLib;
  /*! The memory allocators that the compiler should use. */
  aclMemoryObject_0_9 memory;
} aclCompilerOptions_0_9;

/*!
 * Structure that holds the OpenCL compiler and various loaders.
 */
typedef struct _acl_compiler_rec_0_9 {
  /*! Common structure fields for version checking. */
  ACL_STRUCT_HEADER;
  /*! Pointer to the compiler API. */
  aclCLLoader   clAPI;
  /*! Pointer to the FE Loader API. */
  aclFELoader   feAPI;
  /*! Pointer to the Opt Loader API. */
  aclOptLoader  optAPI;
  /*! Pointer to the Link Loader API. */
  aclLinkLoader linkAPI;
  /*! Pointer to the CG Loader API. */
  aclCGLoader   cgAPI;
  /*! Pointer to the BE Loader API. */
  aclBELoader   beAPI;
  /*! Pointer to the SC Loader API. */
  aclSCLoader   scAPI;
  /*! Pointer to the memory API. */
  aclMemoryObject_0_9  memory;
  /*! The options structure for the compiler. */
  aclCompilerOptions *opts;
  /*! Pointer to the llvm shutdown object. */
  void *llvm_shutdown;
  /*! Pointer to the current build log. */
  char *buildLog;
  /*! Size of the current build log. */
  unsigned    logSize;
  /*! pointer to data store for the compiler API loader. */
  aclLoaderData *apiData;
} aclCompilerHandle_0_9;
/** @} */

typedef enum   _acl_key_value_enum_0_9     aclKeyValue;

#endif // _CL_API_TYPES_0_9_H_
