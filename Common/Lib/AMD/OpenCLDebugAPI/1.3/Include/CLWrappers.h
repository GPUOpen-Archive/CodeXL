//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLWrappers.h $
/// \version $Revision: #8 $
/// \brief  CL wrapper functions that must be called in place
///         of the real CL fucntions to allow additional
///         CL functionality.
//
//=====================================================================
// $Id: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLWrappers.h#8 $
// Last checkin:   $DateTime: 2014/02/24 10:19:29 $
// Last edited by: $Author: ushomron $
// Change list:    $Change: 489910 $
//=====================================================================

#ifndef CL_WRAPPERS_H
#define CL_WRAPPERS_H

#include <CL/cl.h>


#ifdef __cplusplus
extern "C" {
#endif

/// Wrapper used to create a kernel object.
/// \param[in] program       A program object with a successfully built executable.
/// \param[in] kernel_name   A function name in the program declared with the __kernel  qualifier.
/// \param[in] errcode_ret   Returns an appropriate error code. If errcode_ret  is NULL, no error code is returned.
/// \return                  Returns a valid kernel object if the function executed successfully.
///                          Otherwise, it returns NULL.
extern CL_API_ENTRY cl_kernel CL_API_CALL
amdclInterceptCreateKernel( cl_program  program,
                            const char* kernel_name,
                            cl_int*     errcode_ret ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to create kernel objects for all kernel functions in a program object.
/// \param[in] program           A program object with a successfully built executable.
/// \param[in] num_kernels       The size of memory pointed to by kernels specified as the number of cl_kernel entries.
/// \param[in] kernels           The buffer where the kernel objects for kernels in program  will be returned.
/// \param[in] num_kernels_ret   The number of kernels in program. If num_kernels_ret is NULL, it is ignored.
/// \return                      Returns a CL_SUCCESS if the function executed successfully.
///                              Otherwise, it returns an OpenCL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclInterceptCreateKernelsInProgram( cl_program program,
                                      cl_uint    num_kernels,
                                      cl_kernel* kernels,
                                      cl_uint*   num_kernels_ret ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to increment the kernel object reference count.
/// \param[in] kernel        A valid kernel object.
/// \return                  Returns CL_SUCCESS if the function executed successfully.
///                          Otherwise, it returns a CL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclInterceptRetainKernel( cl_kernel kernel ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to decrement the kernel reference count.
/// \param[in] kernel        A valid kernel object.
/// \return                  Returns CL_SUCCESS if the function executed successfully.
///                          Otherwise, it returns a CL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclInterceptReleaseKernel( cl_kernel kernel ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to set the argument value for a specific argument of a kernel.
/// Keeps track of each argument value set for a kernel.
/// \param[in] kernel        A valid kernel object.
/// \param[in] arg_index     The argument index.
/// \param[in] arg_size      Used to specify the size in bytes of memory pointed to by arg_value.
/// \param[in] arg_value     A pointer to data that should be used to store the argument value
///                          for argument specified by arg_index.
/// \param[in] arg_size_ret  The actual size in bytes of data copied to arg_value.
///                          If param_value_size_ret is NULL, it is ignored. 
/// \return                  Returns CL_SUCCESS if the function executed successfully.
///                          Otherwise, it returns a CL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclInterceptSetKernelArg( cl_kernel   kernel,
                            cl_uint     arg_index,
                            size_t      arg_size,
                            const void* arg_value ) CL_API_SUFFIX__VERSION_1_0;

#ifdef __cplusplus
}
#endif

#endif // CL_WRAPPERS_H
