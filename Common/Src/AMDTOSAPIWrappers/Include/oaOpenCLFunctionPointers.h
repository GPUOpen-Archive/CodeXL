//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenCLFunctionPointers.h
///
//=====================================================================

//------------------------------ oaOpenCLFunctionPointers.h ------------------------------

#ifndef __OAOPENCLFUNCTIONPOINTERS
#define __OAOPENCLFUNCTIONPOINTERS

// clCreateContext function type:
typedef cl_context(CL_API_CALL* PFNCLCREATECONTEXTFROMTYPEPROC)(const cl_context_properties*, cl_device_type , void (*pfn_notify)(const char*, const void*, size_t, void*), void*, cl_int*);
typedef cl_int(CL_API_CALL* PFNCLGETPLATFORMIDSPROC)(cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int(CL_API_CALL* PFNCLGETPLATFORMINFOSPROC)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
typedef cl_int(CL_API_CALL* PFNCLGETDEVICEIDSPROC)(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices);
typedef cl_int(CL_API_CALL* PFNCLGETDEVICEINFOPROC)(cl_device_id device, cl_device_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
typedef cl_int(CL_API_CALL* PFNCLGETCONTEXTINFOPROC)(cl_context, cl_context_info , size_t , void*, size_t*);
typedef cl_command_queue(CL_API_CALL* PFNCLCREATECOMMANDQUEUEPROC)(cl_context, cl_device_id, cl_command_queue_properties, cl_int*);

#endif  // __OAOPENCLFUNCTIONPOINTERS
