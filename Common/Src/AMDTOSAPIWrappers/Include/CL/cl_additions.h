//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

#ifndef __OPENCL_CL_ADDITIONS_H
#define __OPENCL_CL_ADDITIONS_H

/*
This file contains additions that are not part of the cl.h file
(either removed from an older version or not yet added to an official version)
*/

#ifdef __cplusplus
extern "C" {
#endif

/********************/
/* OpenCL 1.0 - 1.1 */
/********************/

/********/
/* cl.h */
/********/
extern CL_API_ENTRY cl_int CL_API_CALL
clSetPrintfCallback(cl_context          /* context */,
                    void (CL_CALLBACK* /* pfn_notify */)(cl_context /* program */,
                                                         cl_uint /*printf_data_len */,
                                                         char* /* printf_data_ptr */,
                                                         void* /* user_data */),
                    void*               /* user_data */) /*CL_API_SUFFIX__VERSION_1_2*/;

#ifdef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#pragma message ("NOTE: CL_USE_DEPRECATED_OPENCL_1_0_APIS is defined. These APIs are unsupported and untested in OpenCL 1.1!")
/*
 *  WARNING:
 *     This API introduces mutable state into the OpenCL implementation. It has been REMOVED
 *  to better facilitate thread safety.  The 1.0 API is not thread safe. It is not tested by the
 *  OpenCL 1.1 conformance test, and consequently may not work or may not work dependably.
 *  It is likely to be non-performant. Use of this API is not advised. Use at your own risk.
 *
 *  Software developers previously relying on this API are instructed to set the command queue
 *  properties when creating the queue, instead.
 */
extern CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(cl_command_queue              /* command_queue */,
                          cl_command_queue_properties   /* properties */,
                          cl_bool                        /* enable */,
                          cl_command_queue_properties* /* old_properties */) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED;
#endif /* CL_USE_DEPRECATED_OPENCL_1_0_APIS */


/*******************************/
/* OpenCL 1.0 - 1.1 Extensions */
/*******************************/

/*********************/
/* cl_ext_internal.h */
/*********************/

/* cl_khr_fp64 extension - no extension #define since it has no functions  */
#define CL_DEVICE_DOUBLE_FP_CONFIG                  0x1032

/***********************************
 * cl_ext_migrate_memobject extension definitions
 ***********************************/
#define cl_ext_migrate_memobject 1

typedef cl_bitfield cl_mem_migration_flags_ext;

#define CL_MIGRATE_MEM_OBJECT_HOST_EXT              0x1

#define CL_COMMAND_MIGRATE_MEM_OBJECT_EXT           0x4040

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjectEXT(cl_command_queue /* command_queue */,
                             cl_uint /* num_mem_objects */,
                             const cl_mem* /* mem_objects */,
                             cl_mem_migration_flags_ext /* flags */,
                             cl_uint /* num_events_in_wait_list */,
                             const cl_event* /* event_wait_list */,
                             cl_event* /* event */) CL_EXT_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clEnqueueMigrateMemObjectEXT_fn)(cl_command_queue /* command_queue */,
                                               cl_uint /* num_mem_objects */,
                                               const cl_mem* /* mem_objects */,
                                               cl_mem_migration_flags_ext /* flags */,
                                               cl_uint /* num_events_in_wait_list */,
                                               const cl_event* /* event_wait_list */,
                                               cl_event* /* event */) CL_EXT_SUFFIX__VERSION_1_1;

/****************************/
/* cl_ext_object_metadata.h */
/****************************/

// <amd_internal>
/*************************
* cl_ext_object_metadata *
**************************/
#define cl_ext_object_metadata 1

typedef size_t cl_key_ext;

#define CL_INVALID_OBJECT_EXT    0x403A
#define CL_INVALID_KEY_EXT       0x403B
#define CL_PLATFORM_MAX_KEYS_EXT 0x403C

typedef CL_API_ENTRY cl_key_ext(CL_API_CALL* clCreateKeyEXT_fn)(
    cl_platform_id      /* platform */,
    void (CL_CALLBACK* /* destructor */)(void* /* old_value */),
    cl_int*             /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int(CL_API_CALL* clObjectGetValueForKeyEXT_fn)(
    void*                /* object */,
    cl_key_ext           /* key */,
    void**               /* ret_val */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int(CL_API_CALL* clObjectSetValueForKeyEXT_fn)(
    void*                /* object */,
    cl_key_ext           /* key */,
    void*                /* value */) CL_API_SUFFIX__VERSION_1_1;

/*************
* cl_amd_hsa *
*************/
#define CL_HSA_ENABLED_AMD                          (1ull << 62)
#define CL_HSA_DISABLED_AMD                         (1ull << 63)

/*******************************************
* Shared Virtual Memory (SVM) extension
* The declarations match the order, naming and values of the original 2.0
* standard, except for the fact that we added the _AMD suffix to each
* symbol
*******************************************/
typedef cl_bitfield                      cl_device_svm_capabilities_amd;
typedef cl_bitfield                      cl_svm_mem_flags_amd;
typedef cl_uint                          cl_kernel_exec_info_amd;

/* cl_device_info */
#define CL_DEVICE_SVM_CAPABILITIES_AMD                     0x1053
#define CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT_AMD  0x1054

/* cl_device_svm_capabilities_amd */
#define CL_DEVICE_SVM_COARSE_GRAIN_BUFFER_AMD             (1 << 0)
#define CL_DEVICE_SVM_FINE_GRAIN_BUFFER_AMD               (1 << 1)
#define CL_DEVICE_SVM_FINE_GRAIN_SYSTEM_AMD               (1 << 2)
#define CL_DEVICE_SVM_ATOMICS_AMD                         (1 << 3)

/* cl_svm_mem_flags_amd */
#define CL_MEM_SVM_FINE_GRAIN_BUFFER_AMD                  (1 << 10)
#define CL_MEM_SVM_ATOMICS_AMD                            (1 << 11)

/* cl_mem_info */
#define CL_MEM_USES_SVM_POINTER_AMD                       0x1109

/* cl_kernel_exec_info_amd */
#define CL_KERNEL_EXEC_INFO_SVM_PTRS_AMD                  0x11B6
#define CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM_AMD     0x11B7

/* cl_command_type */
#define CL_COMMAND_SVM_FREE_AMD                           0x1209
#define CL_COMMAND_SVM_MEMCPY_AMD                         0x120A
#define CL_COMMAND_SVM_MEMFILL_AMD                        0x120B
#define CL_COMMAND_SVM_MAP_AMD                            0x120C
#define CL_COMMAND_SVM_UNMAP_AMD                          0x120D

/*******************************
 * cl_khr_sub_groups extension *
 *******************************/
#define cl_khr_sub_groups 1

typedef cl_uint  cl_kernel_sub_group_info;

/* cl_khr_sub_group_info */
// NOTE: these are defined in cl_ext.h without the _KHR suffix
#define CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR    0x2033
#define CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE_KHR       0x2034


extern CL_API_ENTRY cl_int CL_API_CALL
clGetKernelSubGroupInfoKHR(cl_kernel /* in_kernel */,
                           cl_device_id /*in_device*/,
                           const cl_kernel_sub_group_info /* param_name */,
                           size_t /*input_value_size*/,
                           const void* /*input_value*/,
                           size_t /*param_value_size*/,
                           void* /*param_value*/,
                           size_t* /*param_value_size_ret*/) /* CL_EXT_SUFFIX__VERSION_2_0 */;

typedef CL_API_ENTRY cl_int
(CL_API_CALL* clGetKernelSubGroupInfoKHR_fn)(cl_kernel /* in_kernel */,
                                             cl_device_id /*in_device*/,
                                             const cl_kernel_sub_group_info /* param_name */,
                                             size_t /*input_value_size*/,
                                             const void* /*input_value*/,
                                             size_t /*param_value_size*/,
                                             void* /*param_value*/,
                                             size_t* /*param_value_size_ret*/) /* CL_EXT_SUFFIX__VERSION_2_0 */;

/***************************
* cl_amd_command_intercept *
***************************/
#define CL_CONTEXT_COMMAND_INTERCEPT_CALLBACK_AMD   0x403D
#define CL_QUEUE_COMMAND_INTERCEPT_ENABLE_AMD       (1ull << 63)

/**************************
* cl_amd_command_queue_info *
**************************/
#define CL_QUEUE_THREAD_HANDLE_AMD                  0x403E

typedef cl_int(CL_CALLBACK* intercept_callback_fn)(cl_event, cl_int*);

#ifdef __cplusplus
}
#endif

#endif  /* __OPENCL_CL_ADDITIONS_H */
