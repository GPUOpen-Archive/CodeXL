//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLIntercept.h $
/// \version $Revision: #9 $
/// \brief  CL wrapper functions that must be called in place
///         of the real CL fucntions to allow additional
///         CL functionality.
//
//=====================================================================
// $Id: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLIntercept.h#9 $
// Last checkin:   $DateTime: 2014/11/26 12:01:40 $
// Last edited by: $Author: ushomron $
// Change list:    $Change: 511421 $
//=====================================================================

#ifndef CL_WRAPPERS2_H
#define CL_WRAPPERS2_H

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Wrapper used to create a context object.
/// \param[in] properties    Specifies a list of context property names and their corresponding values.
/// \param[in] num_devices   The number of devices specified in the devices argument.
/// \param[in] devices       A pointer to a list of unique devices returned by clGetDeviceIDs for a platform.
/// \param[in] pfn_notify    A callback function that can be registered by the application.
/// \param[in] user_data     Passed as the user_data argument when pfn_notify is called. user_data can be NULL. 
/// \param[in] errcode_ret   Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
/// \return                  Returns a valid context object if the function executed successfully.
///                          Otherwise, it returns NULL.
extern CL_API_ENTRY cl_context CL_API_CALL
amdclInterceptCreateContext( const cl_context_properties* properties,
                             cl_uint                      num_devices,
                             const cl_device_id*          devices,
                             void (CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                             void*                        user_data,
                             cl_int*                      errcode_ret ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to create a context object.
/// \param[in] properties    Specifies a list of context property names and their corresponding values.
/// \param[in] device_type   A bit-field that identifies the type of device and is described in the table below.
/// \param[in] pfn_notify    A callback function that can be registered by the application.
/// \param[in] user_data     Passed as the user_data argument when pfn_notify is called. user_data can be NULL. 
/// \param[in] errcode_ret   Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
/// \return                  Returns a valid context object if the function executed successfully.
///                          Otherwise, it returns NULL.
extern CL_API_ENTRY cl_context CL_API_CALL
amdclInterceptCreateContextFromType( const cl_context_properties* properties,
                                     cl_device_type               device_type,
                                     void (CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                                     void*                        user_data,
                                     cl_int*                      errcode_ret ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to create a command queue.
/// \param[in] context       Must be a valid OpenCL context.
/// \param[in] device        Must be a device associated with context.
/// \param[in] properties    Specifies a list of properties for the command-queue.
/// \param[in] errcode_ret   Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
/// \return                  Returns a valid command queue if the function executed successfully.
///                          Otherwise, it returns NULL.
extern CL_API_ENTRY cl_command_queue CL_API_CALL
amdclInterceptCreateCommandQueue( cl_context                  context,
                                  cl_device_id                device,
                                  cl_command_queue_properties properties,
                                  cl_int*                     errcode_ret ) CL_API_SUFFIX__VERSION_1_0;

/// Wrapper used to create a command queue with queue properties.
/// \param[in] context       Must be a valid OpenCL context.
/// \param[in] device        Must be a device associated with context.
/// \param[in] properties    Specifies a list of properties for the command-queue.
/// \param[in] errcode_ret   Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
/// \return                  Returns a valid command queue if the function executed successfully.
///                          Otherwise, it returns NULL.
extern CL_API_ENTRY cl_command_queue CL_API_CALL
amdclInterceptCreateCommandQueueWithProperties( cl_context                 context,
                                                cl_device_id               device,
                                                const cl_queue_properties* properties,
                                                cl_int*                    errcode_ret ) CL_API_SUFFIX__VERSION_2_0;

#ifdef __cplusplus
}
#endif

#endif // CL_WRAPPERS2_H
