/************************************************************************************//**
** Copyright (c) 2014-2015 Advanced Micro Devices, Inc. All rights reserved.
**
** \author AMD Developer Tools
** \file
** \brief A C interface for HSA debug info (dwarf)
****************************************************************************************/
#ifndef FACILITIES_INTERFACE_H_
#define FACILITIES_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************/
/* HwDbgInfo types:                                         */
/************************************************************/
/* A debug info handle                                      */
typedef const void* HwDbgInfo_debug;
/* A (HLC) code location - file and line number             */
typedef const void* HwDbgInfo_code_location;
/* A single frame in a call stack                           */
typedef const void* HwDbgInfo_frame_context;
/* A line number                                            */
typedef unsigned long long HwDbgInfo_linenum;
/* Debug information for a variable, parameter, or member   */
typedef const void* HwDbgInfo_variable;
/* A memory or instruction address                          */
typedef unsigned long long HwDbgInfo_addr;
/* Error / success code (HWDBGINGO_E_*)                     */
typedef unsigned int HwDbgInfo_err;
/* Variable encoding (HWDBGINFO_VENC_*)                     */
typedef unsigned int HwDbgInfo_encoding;
/* Variable indirection (HWDBGINFO_VIND_*)                  */
typedef unsigned int HwDbgInfo_indirection;
/* Variable indirection detail (HWDBGINFO_VINDD_*)          */
typedef unsigned int HwDbgInfo_indirectiondetail;
/* Variable location register (HWDBGINFO_VLOC_REG_*         */
typedef unsigned int HwDbgInfo_locreg;

/***************/
/* Error codes */
/***************/
/* Success */
#define HWDBGINFO_E_SUCCESS             0
/* Unexpected failure */
#define HWDBGINFO_E_UNEXPECTED          1
/* Invalid parameter */
#define HWDBGINFO_E_PARAMETER           2
/* Supplied output buffer is too small */
#define HWDBGINFO_E_BUFFERTOOSMALL      3
/* Cannot allocate enough memory */
#define HWDBGINFO_E_OUTOFMEMORY         4
/* Binary not found or is invalid */
#define HWDBGINFO_E_NOBINARY            5
/* Kernel cannot be identified */
#define HWDBGINFO_E_NOKERNEL            6
/* HL binary (BRIG DWARF) cannot be found */
#define HWDBGINFO_E_NOHLBINARY          7
/* LL binary (ISA DWARF) cannot be found */
#define HWDBGINFO_E_NOLLBINARY          8
/* Unexpected binary format */
#define HWDBGINFO_E_BINARY              9
/* Invalid HL (BRIG DWARF) debug information */
#define HWDBGINFO_E_HLINFO              10
/* Invalid LL (ISA DWARF) debug information */
#define HWDBGINFO_E_LLINFO              11
/* Requested information cannot be found */
#define HWDBGINFO_E_NOTFOUND            12
/* Requested constant information from a variable or vice-versa */
#define HWDBGINFO_E_VARIABLEVALUETYPE   13
/* Source code not available */
#define HWDBGINFO_E_NOSOURCE            14

/*********************/
/* Variable encoding */
/*********************/
/* Pointer encoding (%p)                    */
#define HWDBGINFO_VENC_POINTER      0
/* Boolean encoding (T/F)                   */
#define HWDBGINFO_VENC_BOOLEAN      1
/* Floating-point encoding (%f/%e/%g)       */
#define HWDBGINFO_VENC_FLOAT        2
/* Signed integer encoding (%d/%l/%ll)      */
#define HWDBGINFO_VENC_INTEGER      3
/* Unsigned integer encoding (%u/%lu/%llu)  */
#define HWDBGINFO_VENC_UINTEGER     4
/* Signed character encoding (%c)           */
#define HWDBGINFO_VENC_CHARACTER    5
/* Unsigned character encoding (%uc)        */
#define HWDBGINFO_VENC_UCHARACTER   6
/* No valid encoding (e.g. struct type)     */
#define HWDBGINFO_VENC_NONE         7

/*************************/
/* Variable indirection: */
/*************************/
/* Direct value (int)       */
#define HWDBGINFO_VIND_DIRECT 0
/* Pointer value (int*)     */
#define HWDBGINFO_VIND_POINTER 1
/* Reference value (int&)   */
#define HWDBGINFO_VIND_REFERENCE 2
/* Array value (int[])      */
#define HWDBGINFO_VIND_ARRAY 3

/*********************************************************************/
/* Variable indirection detail for AMD GPUs = pointer address space: */
/*********************************************************************/
/* Not a pointer                    */
#define HWDBGINFO_VINDD_AMD_GPU_NOT_A_POINTER 0
/* Pointer to global memory         */
#define HWDBGINFO_VINDD_AMD_GPU_GLOBAL_POINTER 1
/* Pointer to region (GDS) memory   */
#define HWDBGINFO_VINDD_AMD_GPU_GDS_POINTER 2
/* Pointer to local (LDS) memory    */
#define HWDBGINFO_VINDD_AMD_GPU_LDS_POINTER 3
/* Pointer to private memory        */
#define HWDBGINFO_VINDD_AMD_GPU_PRIVATE_POINTER 4
/* Pointer to const global memory   */
#define HWDBGINFO_VINDD_AMD_GPU_CONSTANT_POINTER 5
/* Undefined / uninitialized        */
#define HWDBGINFO_VINDD_AMD_GPU_UNKNOWN_POINTER 6

/************************************/
/* Variable location register type: */
/************************************/
/* Value is located in a register       */
#define HWDBGINFO_VLOC_REG_REGISTER 0
/* Value is located at a stack offset   */
#define HWDBGINFO_VLOC_REG_STACK 1
/* Variable located at pure address     */
#define HWDBGINFO_VLOC_REG_NONE 2
/* Undefined / uninitialized            */
#define HWDBGINFO_VLOC_REG_UNINIT 3

/*******************/
/* Initialization: */
/*******************/
/* Create a HwDbgInfo_debug from an HSA 1.0 (May) binary */
HwDbgInfo_debug hwdbginfo_init_with_hsa_1_0_binary(const void* bin, size_t bin_size, HwDbgInfo_err* err);
/* Create a HwDbgInfo_debug directly from the BRIG DWARF container and ISA DWARF container */
HwDbgInfo_debug hwdbginfo_init_with_two_binaries(const void* hl_bin, size_t hl_bin_size, const void* const ll_bin, size_t ll_bin_size, HwDbgInfo_err* err);

/***********************/
/* Binary data access: */
/***********************/
/* Get the HSAIL text source, if it was available */
HwDbgInfo_err hwdbginfo_get_hsail_text(HwDbgInfo_debug dbg, const char** hsail_source, size_t* hsail_source_len);

/*******************/
/* Debug lines API */
/*******************/
/* Create a code location for queries */
HwDbgInfo_code_location hwdbginfo_make_code_location(const char* file_name, HwDbgInfo_linenum line_num);
/* Query a code location for its details */
HwDbgInfo_err hwdbginfo_code_location_details(HwDbgInfo_code_location loc, HwDbgInfo_linenum* line_num, size_t buf_len, char* file_name, size_t* file_name_len);
/* Query a frame context for its details */
HwDbgInfo_err hwdbginfo_frame_context_details(HwDbgInfo_frame_context frm, HwDbgInfo_addr* pc, HwDbgInfo_addr* fp, HwDbgInfo_addr* mp, HwDbgInfo_code_location* loc, size_t buf_len, char* func_name, size_t* func_name_len);

/* Translate a LL address to a HL line */
HwDbgInfo_err hwdbginfo_addr_to_line(HwDbgInfo_debug dbg, HwDbgInfo_addr addr, HwDbgInfo_code_location* loc);
/* Translate a HL line to LL address(es) */
HwDbgInfo_err hwdbginfo_line_to_addrs(HwDbgInfo_debug dbg, HwDbgInfo_code_location loc, size_t buf_len, HwDbgInfo_addr* addrs, size_t* addr_count);
/* Get the nearest legal (mapped) HL line */
HwDbgInfo_err hwdbginfo_nearest_mapped_line(HwDbgInfo_debug dbg, HwDbgInfo_code_location base_line, HwDbgInfo_code_location* line);
/* Get the nearest legal (mapped) LL address */
HwDbgInfo_err hwdbginfo_nearest_mapped_addr(HwDbgInfo_debug dbg, HwDbgInfo_addr base_addr, HwDbgInfo_addr* addr);
/* Get all legal (mapped) LL addresses */
HwDbgInfo_err hwdbginfo_all_mapped_addrs(HwDbgInfo_debug dbg, size_t buf_len, HwDbgInfo_addr* addrs, size_t* addr_count);
/* Get a LL address's virtual (inlined) call stack */
HwDbgInfo_err hwdbginfo_addr_call_stack(HwDbgInfo_debug dbg, HwDbgInfo_addr start_addr, size_t buf_len, HwDbgInfo_frame_context* stack_frames, size_t* frame_count);
/* Get all the addresses that can be the target of a step operation from a LL address */
HwDbgInfo_err hwdbginfo_step_addresses(HwDbgInfo_debug dbg, HwDbgInfo_addr start_addr, bool step_out, size_t buf_len, HwDbgInfo_addr* addrs, size_t* addr_count);

/******************/
/* Debug info API */
/******************/
/* Query a variable / constant for its data */
HwDbgInfo_err hwdbginfo_variable_data(HwDbgInfo_variable var, size_t name_buf_len, char* var_name, size_t* var_name_len, size_t type_name_buf_len, char* type_name, size_t* type_name_len, size_t* var_size, HwDbgInfo_encoding* encoding, bool* is_constant, bool* is_output);
/* Query a variable for its data */
HwDbgInfo_err hwdbginfo_variable_location(HwDbgInfo_variable var, HwDbgInfo_locreg* reg_type, unsigned int* reg_num, bool* deref_value, unsigned int* offset, unsigned int* resource, unsigned int* isa_memory_region, unsigned int* piece_offset, unsigned int* piece_size, int* const_add);
/* Query a constant for its data */
HwDbgInfo_err hwdbginfo_variable_const_value(HwDbgInfo_variable var, size_t buf_size, void* var_value);
/* Query a variable for its indirection data */
HwDbgInfo_err hwdbginfo_variable_indirection(HwDbgInfo_variable var, HwDbgInfo_indirection* var_indir, HwDbgInfo_indirectiondetail* var_indir_detail);
/* Query a variable /constant for its members */
HwDbgInfo_err hwdbginfo_variable_members(HwDbgInfo_variable var, size_t buf_len, HwDbgInfo_variable* members, size_t* member_count);
/* Query a variable for its definition scope */
HwDbgInfo_err hwdbginfo_variable_range(HwDbgInfo_variable var, HwDbgInfo_addr* loPC, HwDbgInfo_addr* hiPC);

/* Get a (HL to LL) variable information by name */
HwDbgInfo_variable hwdbginfo_variable(HwDbgInfo_debug dbg, HwDbgInfo_addr start_addr, bool current_scope_only, const char* var_name, HwDbgInfo_err* err);
/* Get a LL variable information by name */
HwDbgInfo_variable hwdbginfo_low_level_variable(HwDbgInfo_debug dbg, HwDbgInfo_addr start_addr, bool current_scope_only, const char* var_name, HwDbgInfo_err* err);
/* Get all variables defined in the scope of a LL address */
HwDbgInfo_err hwdbginfo_frame_variables(HwDbgInfo_debug dbg, HwDbgInfo_addr start_addr, int stack_depth, bool leaf_members, size_t buf_len, HwDbgInfo_variable* vars, size_t* var_count);

/***************************/
/* Release allocated data: */
/***************************/
void hwdbginfo_release_debug_info(HwDbgInfo_debug* dbg);
void hwdbginfo_release_code_locations(HwDbgInfo_code_location* locs, size_t loc_count);
void hwdbginfo_release_frame_contexts(HwDbgInfo_frame_context* frames, size_t frame_count);
void hwdbginfo_release_variables(HwDbgInfo_debug dbg, HwDbgInfo_variable* vars, size_t var_count);

#ifdef __cplusplus
}
#endif

#endif /* FACILITIES_INTERFACE_H_ */
