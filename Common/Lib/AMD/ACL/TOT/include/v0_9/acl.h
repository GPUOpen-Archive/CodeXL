//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _ACL_0_9_H_
#define _ACL_0_9_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "aclTypes.h"
/** \defgroup aclAPI09 Functions in the version 0.9 API.
 * This group represents the entire version 0.9 API.
 */
/** \defgroup aclCompiler Functions that deal with aclCompiler objects.
 * The group of functions that create/modify/query/destroy aclCompiler objects.
 * @{
 */

  /**
   * @brief Create an aclCompiler object based on the compiler options.
   * @arg opts Option structure that holds some configuration details about
   * the compiler.
   * @arg error_code [OUT] Pointer to an acl_error object that stores the
   * resulting value.
   * @return object on success or null on error. error_code variable will
   * hold the error that occured or ACL_SUCCESS if no error occured.
   *
   * Creates an aclCompiler object based on the aclCompilerOptions structure
   * and correctly returns based on the options that are passed in.
   */
aclCompiler* ACL_API_ENTRY
aclCompilerInit(aclCompilerOptions *opts, acl_error *error_code) ACL_API_0_9;

/**
 * @brief Cleanup all memory allocated with the compiler.
 * @arg cl Compile to cleanup memory for.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclCompilerFini(aclCompiler *cl) ACL_API_0_9;

/**
 * @brief Get the version of the compiler.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return returns the CL Version enum that corresponds to the compiler
 * version.
 */
aclCLVersion ACL_API_ENTRY
  aclCompilerVersion(aclCompiler *cl, acl_error *error_code) ACL_API_0_9;

/**
 * @brief Get the compiler log for a specific cl object.
 * @arg cl The compiler object that holds the compiler log.
 * @return Returns either NULL on no log or a pointer to the compiler log.
 */
const char* ACL_API_ENTRY
  aclGetCompilerLog(aclCompiler *cl) ACL_API_0_9;

/** @} */

/** \defgroup aclInfo Functions that deal with object versions/errors.
 * The group of functions that return version or error information.
 *  @{
 */

/**
 * @brief Get the size of the compiler version in bytes.
 * @arg num The version number to get the compiler size of.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 */
uint32_t ACL_API_ENTRY
  aclVersionSize(aclCLVersion num, acl_error *error_code) ACL_API_0_9;

/**
 * @brief Converts from an error code to a human readable string.
 * @arg error_code The error code to get a readable string of.
 * @return A pointer to the string representation of the error_code.
 */
const char* ACL_API_ENTRY
  aclGetErrorString(acl_error error_code) ACL_API_0_9;

/**
 * @brief Return the bif version of the specified binary.
 * @arg binary binary to get the bif version of.
 * @return returns the bif version of the binary or aclBIFVersionError on
 * invalid binary.
 */
aclBIFVersion ACL_API_ENTRY
  aclBinaryVersion(const aclBinary *binary) ACL_API_0_9;

/**
 * @brief Get the version string of the DLL type for the compiler.
 * @arg cl Compiler object to get version information from.
 * @arg dll_type loader to get version information of.
 * @arg error_code The error code to get a readable string of.
 * @return The version string of the compiler or loader or NULL on error.
 */
const char*
  aclGetVersionString(const aclCompiler *cl, aclLoaderType dll_type,
      acl_error *error_code) ACL_API_0_9;
/** @} */

/** @defgroup Targets Functions that deal with target specific information
 * @{
 */


/**
 * Returns in the arch_names argument, if non-NULL, a pointer to each of the
 * arch names that the compiler supports. If names is NULL and arch_size is
 * non-NULL, returns the number of arch entries that are required.
 *
 * @brief Get the names of all of the architectures that the compiler library
 * supports.
 * @arg names [OUT] returns 'arch_size' number of strings that have the
 * architecture names .
 * @arg arch_size [OUT] returns the number of architecture names that the
 * compiler library supports.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclGetArchInfo(const char** arch_names,
      size_t *arch_size) ACL_API_0_9;


/**
 *
 * Returns in the names argument, if non-NULL, a pointer to each device
 * name that the compiler supports. If device_size is non-NULL,
 * returns the number of device entries that are used.
 * @brief Get the names of all of the devices for the specified family that the
 * compiler library supports.
 * @arg family The family to retrieve the device names for.
 * @arg names [OUT] returns 'device_size' number of strings that have the
 * device names .
 * @arg device_size [OUT] returns the number of device names that the
 * compiler library supports.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclGetDeviceInfo(const char* arch,
      const char **names,
      size_t *device_size) ACL_API_0_9;


/**
 * Function that returns a correctly filled out aclTargetInfo structure based
 * on the information passed into the kernel.
 * @brief Given an architecture and device combination, returns a target
 * information structure.
 * @arg arch The architecture to select from.
 * @arg chip The specific chip to select.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns an empty aclTargetInfo struct and fills in the error code in
 * the error_code value if it is non-null, otherwise returns a valid
 * aclTargetInfo struct on success.
 */
aclTargetInfo ACL_API_ENTRY
  aclGetTargetInfo(const char *arch,
      const char *device,
      acl_error *error_code) ACL_API_0_9;


/**
 *
 * Function that returns a string representation of the target architecture.
 * @brief Function that returns a string representation of the target
 * architecture.
 * @arg target Target info structure to get the architecture name for.
 * @return Retrieve the architecture name for the target or "unknown" on
 * invalid target.
 */
const char* ACL_API_ENTRY
  aclGetArchitecture(const aclTargetInfo &target) ACL_API_0_9;

//! Function that returns a string representation of the target family.

/**
 * @brief Function that returns a string representation of the target family.
 * @arg target Target info structure to get the family name for.
 * @return Retrieve the family name for the target or "unknown" on invalid
 * target.
 */
const char* ACL_API_ENTRY
  aclGetFamily(const aclTargetInfo &target) ACL_API_0_9;

//! Function that returns a string representation of the target chip.

/**
 * @brief Function that returns a string representation of the target chip.
 * @arg target Target info structure to get the chip name for.
 * @return Retrieve the chip name for the target or "unknown" on invalid
 * target.
 */
const char* ACL_API_ENTRY
  aclGetChip(const aclTargetInfo &target) ACL_API_0_9;

/** @} */

/** @defgroup Binaries Functions that deal with aclBinary objects
 * @{
 * Functions that deal with the creation, modification and deletion of
 * aclBinary objects.
 */
/**
 * @brief Generate a aclBinary target for the specific version of the binary,
 * target and with the given options.
 * @arg binary_version The size of the struct for the binary version that
 * should be created, usually sizeof(aclBinary).
 * @arg target The target the binary should be created for.
 * @arg options The options to create the binary with.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the device binary.
 */
aclBinary* ACL_API_ENTRY
  aclBinaryInit(
      size_t struct_version,
      const aclTargetInfo *target,
      const aclBinaryOptions *options,
      acl_error *error_code) ACL_API_0_9;


/**
 * @brief Cleanup and destroy the aclBinary object
 * @arg bin The binary object to be cleaned up.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclBinaryFini(aclBinary *bin) ACL_API_0_9;

/**
 * @brief Read an aclBinary object from an ELF file.
 * @arg str File to read the aclBinary from.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the device binary.
 */
aclBinary* ACL_API_ENTRY
  aclReadFromFile(const char *str,
      acl_error *error_code) ACL_API_0_9;

/**
 * @brief Read an aclBinary from an ELF object stored in memory.
 * @arg mem pointer to memory to read the aclBinary from.
 * @arg size size of the memory.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the device binary.
 */
aclBinary* ACL_API_ENTRY
  aclReadFromMem(void *mem,
      size_t size, acl_error *error_code) ACL_API_0_9;

/**
 * @brief Write the aclBinary to a file in ELF format.
 * @arg bin the binary to write to the file.
 * @arg str the name of the file to write the binary to.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclWriteToFile(aclBinary *bin,
      const char *str) ACL_API_0_9;

/**
 * @brief Write the aclBinary to a chunk of memory. The resulting memory must
 * be free'd by the application.
 * @arg bin binary to write to memory
 * @arg mem [OUT] pointer that holds the resulting memory
 * @arg size [OUT] the size of the resulting memory.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclWriteToMem(aclBinary *bin,
      void **mem, size_t *size) ACL_API_0_9;

/**
 * @brief Generate a copy of the input binary and convert it to the correct BIF
 * version.
 * @arg binary binary to make a copy of.
 * @arg version version of the aclBinary to generate.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the aclBinary object.
 */
aclBinary* ACL_API_ENTRY
  aclCreateFromBinary(const aclBinary *binary,
      aclBIFVersion version) ACL_API_0_9;

/**
 * @brief Insert a section into the binary.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to insert the section  and data into.
 * @arg data the data that goes into the section.
 * @arg data_size the size of the data to insert into the section.
 * @arg id The section id to insert the data into.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclInsertSection(aclCompiler *cl,
      aclBinary *binary,
      const void *data,
      size_t data_size,
      aclSections id) ACL_API_0_9;

/**
 * @brief Insert data into a section at a specific symbol.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to insert the symbol and data into.
 * @arg data the data that goes into binary and the symbol points to.
 * @arg data_size the size of the data to insert.
 * @arg id the section to insert the data into.
 * @arg symbol the symbol name that corresponds to the data.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclInsertSymbol(aclCompiler *cl,
      aclBinary *binary,
      const void *data,
      size_t data_size,
      aclSections id,
      const char *symbol) ACL_API_0_9;

/**
 * @brief Insert label into a section at a specified offset.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to insert the label into.
 * @arg id the section to insert the data into.
 * @arg label the label to insert into the section.
 * @arg offset the offset into the section where the label should point to.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclInsertLabel(aclCompiler *cl,
      aclBinary *binary,
      aclSections id,
      const char *label,
      size_t offset) ACL_API_0_9;
/**
 * @brief Return a pointer to the data of a section in the binary. The memory
 * is owned by the binary and should not be freed by the app.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary the binary to get the section data from.
 * @arg size [OUT] the size of the data that is returned.
 * @arg id the section to retrieve data from.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the data in the binary.
 */
const void* ACL_API_ENTRY
  aclExtractSection(aclCompiler *cl,
      const aclBinary *binary,
      size_t *size,
      aclSections id,
      acl_error *error_code) ACL_API_0_9;

/**
 * @brief Extract data from a binary based on the section and symbol. The data
 * is owned by the binary object, so it should not be freed by the application.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to extract symbol data from
 * @arg size [OUT] the size of the data that was returned.
 * @arg id the section the symbol should exist in.
 * @arg symbol the symbol that points to the data that should be retrieved.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the data in the binary.
 *
 * The difference between aclExtractLabel and aclExtractSymbol is that
 * a symbol ALWAYS points to the beginning of the memory it is associated
 * with, whereas a label can point to any offset into a section.
 */
const void* ACL_API_ENTRY
  aclExtractSymbol(aclCompiler *cl,
      const aclBinary *binary,
      size_t *size,
      aclSections id,
      const char *symbol,
      acl_error *error_code) ACL_API_0_9;

/**
 * @brief Extract data from a binary based on the section and label. The data
 * is owned by the binary object, so it should not be freed by the application.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to extract label data from
 * @arg size [OUT] the size of the data that was returned.
 * @arg id the section the label should exist in.
 * @arg label the label that points to the data that should be retrieved.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the data in the binary.
 *
 * The difference between aclExtractLabel and aclExtractSymbol is that
 * a symbol ALWAYS points to the beginning of the memory it is associated
 * with, whereas a label can point to any offset into a section.
 */
const void* ACL_API_ENTRY
  aclExtractLabel(aclCompiler *cl,
      const aclBinary *binary,
      size_t *size,
      aclSections id,
      const char *label,
      acl_error *error_code) ACL_API_0_9;

/**
 * @brief Arguments
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to remove the section from.
 * @arg id section to remove from the binary.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclRemoveSection(aclCompiler *cl,
      aclBinary *binary,
      aclSections id) ACL_API_0_9;

/**
 * @brief Remove the specified symbol and its underlying data from the binary.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to remove the symbol from.
 * @arg id section that the symbol exists in.
 * @arg symbol symbol to remove from the binary.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclRemoveSymbol(aclCompiler *cl,
      aclBinary *binary,
      aclSections id,
      const char *symbol) ACL_API_0_9;

/**
 * @brief Remove the specified label from the binary but leave the data intact.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary binary to remove the label from.
 * @arg id section that the label exists in.
 * @arg label label to remove from the binary.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclRemoveLabel(aclCompiler *cl,
      aclBinary *binary,
      aclSections id,
      const char *label) ACL_API_0_9;

/** @} */

/** @defgroup Debug Functions that deal with debugging
 * @{
 */

/**
 * @brief The function aclDbgAddArgument tells the compiler to create and setup
 * a debug argument for the specific kernel.
 * @arg cl the compiler object that holds the interface functors.
 * @arg bin The binary to add the debug argument to.
 * @arg kernel the name of the kernel to add debug argument to.
 * @arg name the name fo the argument.
 * @arg byval flag to specify if the argument is byval.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 *
 * The function aclDbgAddArgument tells the compiler to create and setup
 * a debug argument for the specific kernel. that is specified in the binary.
 * The argument name will be encoded as ‘__\<kernel\>_\<name\>_dbg’ and will be
 * added after the last user argument to the kernel. The function will return
 * true on success and false on failure. The buffer will be added as a 128 bit
 * data type, or a pointer to 32bit integer. Put simply what this function does
 * is reserve one constant buffer slot and marks it as either a pass by value
 * argument or a pointer argument.
 */
acl_error ACL_API_ENTRY
  aclDbgAddArgument(aclCompiler *cl,
      aclBinary *binary,
      const char* kernel,
      const char* name,
      bool byVal)
  ACL_API_0_9;

/**
 * @brief The function aclDbgRemoveArgument tells the compiler to clear the
 * debug buffer.
 * @arg cl the compiler object that holds the interface functors.
 * @arg bin The binary to remove the debug argument to.
 * @arg kernel the name of the kernel to remove debug argument to.
 * @arg name the name fo the argument.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 *
 * The function aclDbgRemoveArgument tells the compiler to clear the debug
 * buffer that is associated with the specific kernel. The function erases the
 * debug buffer for the kernel if it exists and returns true and false on
 * failure.
 */
acl_error ACL_API_ENTRY
  aclDbgRemoveArgument(aclCompiler *cl,
      aclBinary *binary,
      const char* kernel,
      const char* name)
  ACL_API_0_9;
/** @} */

/** @defgroup Compilation Functions that deal with compilation or compilation
 * results.
 * @{
 */
/**
 * @brief Compile the binary from the 'from' stage of compilation to  the 'to'
 * stage of compilation.
 * @arg cl the compiler object that holds the interface functors.
 * @arg bin the binary that holds the data to be compiled.
 * @arg options the options that the code should be compiled with.
 * @arg from The compilation stage the compiler should start at.
 * @arg to The compilation stage the compiler should end at.
 * @arg compile_callback Returns all the error messages to a user provide
 * functor.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclCompile(aclCompiler *cl,
      aclBinary *bin,
      const char *options,
      aclType from,
      aclType to,
      aclLogFunction compile_callback) ACL_API_0_9;

/**
 * @brief Link the 'num_libs' binaries that are pointed to by 'libs' into the
 * 'src_bin' based on the link_mode. Only ACL_TYPE_LLVMIR_BINARY and
 * ACL_TYPE_HSAIL_BINARY are supported.
 * @arg cl the compiler object that holds the interface functors.
 * @arg src_bin The binary that holds the reference appointment.
 * @arg num_libs Number of binaries to extract libraries from.
 * @arg libs array of binaries that need to be linked into the src_binary.
 * @arg link_mode the type that the libraries are to be linked with.
 * @arg options the options that the code should be compiled with.
 * @arg link_callback Returns all the error messages to a user provide
 * functor.
 * @return ACL_SUCCESS if linking had no errors, otherwise an error code is
 * returned.
 */
acl_error ACL_API_ENTRY
  aclLink(aclCompiler *cl,
      aclBinary *src_bin,
      unsigned int num_libs,
      aclBinary **libs,
      aclType link_mode,
      const char *options,
      aclLogFunction link_callback) ACL_API_0_9;

/**
 * @brief Query information about a kernel from the binary.
 * @arg cl compiler object that holds the interface functors.
 * @arg binary The binary to query information from.
 * @arg query The query for the type of information.
 * @arg kernel The name of the kernel to query information for.
 * @arg data_ptr [OUT] pointer to the resulting data after it is filled in by
 * the function.
 * @arg ptr_size [OUT] Size of the data that needs to have space allocated
 * for.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 *
 * See aclQueryKeys for the various keys and the type of information they
 * retrieve from the binary.
 */
acl_error ACL_API_ENTRY
  aclQueryInfo(aclCompiler *cl,
      const aclBinary *binary,
      aclQueryType query,
      const char *kernel,
      void *data_ptr,
      size_t *ptr_size) ACL_API_0_9;

/**
 * @brief Disassemble the binary for the given kernel and store it in the
 * binary.
 * @arg cl The compiler object that holds the compiler log.
 * @arg bin The binary to disassemble the kernel from.
 * @arg kernel The kernel to disassemble.
 * @arg disasm_callback If specified, the disassembled code will be passed to
 * this functor.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 */
acl_error ACL_API_ENTRY
  aclDisassemble(aclCompiler *cl,
      aclBinary *bin,
      const char *kernel,
      aclLogFunction disasm_callback) ACL_API_0_9;

/** \defgroup Deprecated Functions that have currently been deprecated.
 * @{
 */
/**
 * @brief A helper function that returns the data for the specified type
 * option. This is a wrapper around aclExtractSymbol/Section. The memory is
 * owned by the binary object.
 * @arg cl compiler object that holds the interface functors.
 * @arg bin binary to retrieve data from.
 * @arg name name of the data to retrieve if required.
 * @arg data_size [OUT] size of the data that is returned.
 * @arg type The data type to retrieve.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the data in the binary.
 * @deprecated use aclGetSection/aclGetSymbol.
 *
 */
const void* ACL_API_ENTRY
  aclRetrieveType(aclCompiler *cl,
      const aclBinary *bin,
      const char *name,
      size_t *data_size,
      aclType type,
      acl_error *error_code) ACL_API_0_8;

/**
 * @brief A helper function that sets data based on the type. This is a wrapper
 * around aclInsertSymbol/Section.
 * @arg cl compiler object that holds the interface functors.
 * @arg bin binary to insert the data in.
 * @arg name Name of the data you want to insert.
 * @arg type The data type to insert.
 * @arg data the data to insert.
 * @arg size the size of the data to insert.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 * @deprecated use aclInsertSection/aclInsertSymbol.
 */
acl_error ACL_API_ENTRY
  aclSetType(aclCompiler *cl,
      aclBinary *bin,
      const char *name,
      aclType type,
      const void *data,
      size_t size) ACL_API_0_9;

/**
 * @brief Convert certain data from binary to text format and vice versa.
 * @arg cl compiler object that holds the interface functors.
 * @arg bin the binary to do the conversion in.
 * @arg name the name of the symbol to convert if required.
 * @arg type the data type to do the conversion on.
 * @return ACL_SUCCESS if the function had no errors, otherwise an error code
 * is returned.
 * @deprecated use aclCompile for the *_TEXT <==> *_BINARY compilation options.
 */
acl_error ACL_API_ENTRY
  aclConvertType(aclCompiler *cl,
      aclBinary *bin,
      const char *name,
      aclType type) ACL_API_0_9;

/**
 * @brief Retrieve the device binary so that it can be executed directly. This
 * is a helper function that returns the data in the .text section for the
 * specific kernel. The memory is owned by the binary object.
 * @arg cl compiler object that holds the interface functors.
 * @arg bin the binary that contains the device executable.
 * @arg kernel the name of the kernel to retrieve the device binary for.
 * @arg size [OUT] the size of the data that is returned.
 * @arg error_code [OUT] Pointer to an acl_error object that stores the
 * resulting value.
 * @return Returns NULL and fills in the error code in the error_code value if
 * it is non-null, otherwise returns a pointer to the device binary.
 * @deprecated use aclQueryInfo to retrieve the device binary.
 */
const void* ACL_API_ENTRY
  aclGetDeviceBinary(aclCompiler *cl,
      const aclBinary *bin,
      const char *kernel,
      size_t *size,
      acl_error *error_code) ACL_API_0_9;
/** @} */
/** @} */

/** @defgroup aclSupport Functions to support compiler library development.
 * @{
 */
/**
 * @brief Dump out information about an acl Binary.
 * @arg bin The aclBinary that information needs to be dumped for.
 * @return None
 */
void aclDumpBinary(const aclBinary *bin);
/** @} */
/** @} */
#ifdef __cplusplus
}
#endif
#endif // _ACL_0_9_H_
