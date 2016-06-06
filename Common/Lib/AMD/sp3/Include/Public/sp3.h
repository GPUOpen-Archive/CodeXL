#ifndef SP3_H
#define SP3_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sp3-vm.h"
#include "sp3-type.h"

/// @file sp3.h
/// @brief sp3 API

/// @brief Get version of the sp3 library.
///
/// @return String containing the version number.
///
SP3_EXPORT const char *sp3_version(void);

/// @brief Create a new sp3 context.
///
SP3_EXPORT struct sp3_context *sp3_new(void);

/// @brief Set option for sp3.
///
/// @param state sp3 context.
/// @param option Option name. Unknown options will raise an error.
/// @param value Option value. NULL is used to represent value-less options.
///
SP3_EXPORT void sp3_set_option(struct sp3_context *state, const char *option, const char *value);

/// @brief Parse a file into a context.
///
/// If 'file' is NULL, parse stdin.
///
SP3_EXPORT void sp3_parse_file(struct sp3_context *state, const char *file);

/// @brief Parse a string into a context.
///
SP3_EXPORT void sp3_parse_string(struct sp3_context *state, const char *string);

/// @brief Parse a file from the standard library into a context.
///
SP3_EXPORT void sp3_parse_library(struct sp3_context *state, const char *name);

/// @brief Call a sp3 function.
///
SP3_EXPORT void sp3_call(struct sp3_context *state, const char *func);

/// @brief Call a sp3 CF clause.
///
/// @param state sp3 context.
/// @param cffunc Name of clause to call. By convention, this is "main".
///
/// @return A compiled and linked shader.  Free memory with sp3_free().
///
SP3_EXPORT struct sp3_shader *sp3_compile(struct sp3_context *state, const char *cffunc);

/// @brief Free a sp3_shader.
///
SP3_EXPORT void sp3_free_shader(struct sp3_shader *sh);

/// @brief Get current ASIC name set for a context.
///
SP3_EXPORT const char *sp3_getasic(struct sp3_context *state);

/// @brief Set current ASIC name for a context.
///
SP3_EXPORT void sp3_setasic(struct sp3_context *state, const char *chip);

/// @brief Set global variable in context to an integer.
///
SP3_EXPORT void sp3_set_param_int(struct sp3_context *state, const char *name, int value);

/// @brief Set global variable in context to an integer vector.
///
SP3_EXPORT void sp3_set_param_intvec(struct sp3_context *state, const char *name, int size, const int *value);

/// @brief Set global variable in context to a float.
///
SP3_EXPORT void sp3_set_param_float(struct sp3_context *state, const char *name, float value);

/// @brief Set global variable in context to a float vector.
///
SP3_EXPORT void sp3_set_param_floatvec(struct sp3_context *state, const char *name, int size, const float *value);

/// @brief Set error message header.
///
SP3_EXPORT void sp3_set_error_header(struct sp3_context *state, const char *str);

/// @brief Get ASIC metrics for the ASIC in current state.
///
/// Used by ELF tools to fill in some CAL fields.
///
SP3_EXPORT int sp3_asicinfo(struct sp3_context *state, const char *name);

/// @brief Free a context allocated by sp3_new/open/parse.
///
SP3_EXPORT void sp3_close(struct sp3_context *state);

/// @brief Disassemble a shader.
///
/// This call is likely to change to something that will take a filled sp3_shader structure later on.
///
/// @param state sp3 context (use sp3_new to allocate and sp3_setasic to set ASIC).
/// @param bin Memory map with the opcodes (see sp3-vm.h).
/// @param base Start of the shader in the memory map (in VM entries, i.e. 32-bit words).
/// @param name Same to give the disassembled shader.
/// @param shader_type One of the SHTYPE_* constants.
/// @param include Literal text to include in the CF clause (NULL includes nothing).
/// @param max_len Maximum length of CF clause. Matters if SP3DIS_FORCEVALID is set.
/// @param flags A mask of SP3DIS_* flags.
///
/// @return Shader disassembly as a string (allocated with malloc()).  Free memory with sp3_free().
///
SP3_EXPORT char *sp3_disasm(struct sp3_context *state, sp3_vma *bin, sp3_vmaddr base, const char *name, int shader_type, const char *include, unsigned max_len, unsigned flags);

/// @brief Disassemble a single shader instruction.
///
/// This call is likely to change to something that will take a filled sp3_shader structure later on.
///
/// @param state sp3 context (use sp3_new to allocate and sp3_setasic to set ASIC).
/// @param inst Pointer to dwords containing instruction (exact number of dwords required depends on instruction).
/// @param base Start of the shader in the memory map (in VM entries, i.e. 32-bit words).
/// @param addr Address of the instruction being disassembled (in VM entries, i.e. 32-bit words).
/// @param shader_type One of the SHTYPE_* constants.
/// @param flags A mask of SP3DIS_* flags.
///
/// @return Shader disassembly as a string (allocated with malloc()).  Free memory with sp3_free().
///
SP3_EXPORT char *sp3_disasm_inst(struct sp3_context *state, const unsigned inst[2], sp3_vmaddr base, sp3_vmaddr addr, int shader_type, unsigned flags);

/// @brief Parse a register stream.
///
/// Can be called before sp3_disasm to preset things like ALU, boolean and loop constants.
///
/// This call is likely to merge with sp3_disasm later on.
///
/// @param state sp3 context to fill with state.
/// @param nregs Number of register entries.
/// @param regs Register stream to parse.
/// @param shader_type One of the SHTYPE_* constants.
///
SP3_EXPORT void sp3_setregs(struct sp3_context *state, unsigned nregs, const struct sp3_reg *regs, int shader_type);


/// @brief Set shader comments
///
/// @param state sp3 context.
/// @param map Map of comments (0 for no comment, other values will be passed to the callback).
/// @param f_top Callback returning comment to place above the opcode.
/// @param f_right Callback returning comment to place to the right of the opcode.
/// @param ctx Void pointer to pass to comment callbacks.
///
SP3_EXPORT void sp3_setcomments(struct sp3_context *state, sp3_vma *map, sp3_comment_cb f_top, sp3_comment_cb f_right, void *ctx);

/// @brief Set alternate shader entry points
///
/// Used for disassembly; this marks an additional location in memory
/// (besides the start address) where shader code may be found. Generally
/// required for jump tables and any case where the shader may perform
/// indirect jumps to ensure that disassembly locates all shader
/// instructions.
///
/// @param state sp3 context (use sp3_new to allocate and sp3_setasic to set ASIC).
/// @param addr Address of the instruction being disassembled (in VM entries, i.e. 32-bit words).
///
SP3_EXPORT void sp3_setentrypoint(struct sp3_context *state, sp3_vmaddr addr);

/// @brief Clear alternate shader entry points
///
/// Clear all entry points previously set with sp3_setentrypoint.
///
/// @param state sp3 context (use sp3_new to allocate and sp3_setasic to set ASIC).
///
SP3_EXPORT void sp3_clearentrypoints(struct sp3_context *state);

/// @brief Free memory allocated by sp3.
///
/// Windows DLLs that allocate memory have to free it. This function
/// should be used to free the result of sp3_disasm, sp3_compile etc.
///
SP3_EXPORT void sp3_free(void *ptr);

#ifdef __cplusplus
}
#endif


#endif
