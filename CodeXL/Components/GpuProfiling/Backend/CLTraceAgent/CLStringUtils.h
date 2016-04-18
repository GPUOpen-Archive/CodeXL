//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions to stringify CL params.
//==============================================================================

#ifndef _CL_STRING_UTILS_H_
#define _CL_STRING_UTILS_H_

/// \defgroup CLStringUtils CLStringUtils
/// This module deals with various CL objects to strings
///
/// \ingroup CLTraceAgent
// @{

#include <sstream>
#include <string>
#include <vector>
#include <CL/opencl.h>

#ifdef _WIN32
    #include <CL/cl_d3d11.h>
    #include <CL/cl_d3d10.h>
    #include <CL/cl_dx9_media_sharing.h>
#endif
#include "../CLCommon/CLFunctionDefs.h"

namespace CLStringUtils
{

/// Convert cl_bool to string
/// \param b boolean var
/// \return string representation of the input
std::string GetBoolString(const cl_bool b);

/// Convert ND array to string
/// \param sizes sizes array
/// \param count num of items in sizes
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetSizeListString(const size_t* sizes, size_t count, bool include_brackets = true);

/// Convert ND array to string
/// \param nd N dim array
/// \param dims num of dim
/// \return string representation of the input
std::string GetNDimString(const size_t* nd, size_t dims);

/// Convert cl error code to string
/// \param errcode cl error code
/// \return string representation of the input
std::string GetErrorString(const cl_int errcode);

/// Convert mem obj type to string
/// Could be the following type
/// CL_MEM_OBJECT_BUFFER
/// CL_MEM_OBJECT_IMAGE2D
/// CL_MEM_OBJECT_IMAGE3D
/// CL_MEM_OBJECT_IMAGE2D_ARRAY
/// CL_MEM_OBJECT_IMAGE1D
/// CL_MEM_OBJECT_IMAGE1D_ARRAY
/// CL_MEM_OBJECT_IMAGE1D_BUFFER
/// \param type mem obj type
/// \return string representation of the input
std::string GetMemObjectTypeString(const cl_mem_object_type type);

/// Convert mem info enum to string
/// CL_MEM_TYPE
/// CL_MEM_FLAGS
/// CL_MEM_SIZE
/// CL_MEM_HOST_PTR
/// CL_MEM_MAP_COUNT
/// CL_MEM_REFERENCE_COUNT
/// CL_MEM_CONTEXT
/// CL_MEM_ASSOCIATED_MEMOBJECT
/// CL_MEM_OFFSET
/// \param param_name mem_info
/// \return string representation of the input
std::string GetMemInfoString(const cl_mem_info param_name);

/// Get memory object info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetMemInfoValueString(const cl_mem_info param_name, const void* param_value, cl_int ret_val);

/// Convert image info enum to string
/// CL_IMAGE_FORMAT
/// CL_IMAGE_ELEMENT_SIZE
/// CL_IMAGE_ROW_PITCH
/// CL_IMAGE_SLICE_PITCH
/// CL_IMAGE_WIDTH
/// CL_IMAGE_HEIGHT
/// CL_IMAGE_DEPTH
/// CL_IMAGE_ARRAY_SIZE
/// CL_IMAGE_BUFFER
/// CL_IMAGE_NUM_MIP_LEVELS
/// CL_IMAGE_NUM_SAMPLES
/// \param param_name image info enum
/// \return string representation of the input
std::string GetImageInfoString(const cl_image_info param_name);

/// Get image info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetImageInfoValueString(const cl_image_info param_name, const void* param_value, cl_int ret_val);

/// Convert cl_pipe_info enum to string
/// \param param_name image info enum
/// \return string representation of the input
std::string GetPipeInfoString(const cl_pipe_info param_name);

/// Get cl_pipe_info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetPipeInfoValueString(const cl_pipe_info param_name, const void* param_value, cl_int ret_val);

/// Convert error code pointer to string
/// \param errcode_ret error code pointer
/// \param errcode_retVal the value of the dereferenced pointer
/// \return string representation of the input
std::string GetErrorString(const cl_int* errcode_ret, const cl_int errcode_retVal);

/// Convert an array of error code pointers to string
/// \param errcode_ret an array of error codes
/// \param num_errcodes the number of error codes in the array
/// \return string representation of the input
std::string GetErrorStrings(const cl_int* errcode_ret, cl_uint num_errcodes);

/// Convert an array of pointers to string
/// \param handles pointer array
/// \param num_handles size of the array
/// \return string representation of the input
std::string GetHandlesString(const void* handles, cl_uint num_handles);

/// Convert context property to string
/// \param cprop property type
/// \return string representation of the input
std::string GetContextPropertyString(const cl_context_properties cprop);

/// Convert context property array to string
/// \param pProperties the raw properties pointer
/// \param properties property vector
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetContextPropertiesString(const cl_context_properties* pProperties,
                                       const std::vector<cl_context_properties>& properties,
                                       bool include_brackets = true);

/// Convert command queue properties to string
/// CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
/// CL_QUEUE_PROFILING_ENABLE
/// \param property property flag
/// \return string representation of the input
std::string GetCommandQueuePropertyString(const cl_command_queue_properties property);

/// Convert command queue property array to string
/// \param pProperties the raw properties pointer
/// \param property the properties value
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetCommandQueuePropertiesString(const cl_command_queue_properties* pProperties, const cl_command_queue_properties property, bool include_brackets = true);

/// Convert command queue property array to string
/// \param pProperties the raw properties pointer
/// \param properties the properties values
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetCommandQueuePropertiesString(const cl_queue_properties* pProperties, std::vector<cl_queue_properties> properties, bool include_brackets = true);

/// Convert pipe property to string
/// \param prop property type
/// \return string representation of the input
std::string GetPipePropertyString(const cl_pipe_properties prop);

/// Convert pipe property array to string
/// \param pProperties the raw properties pointer
/// \param properties property vector
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetPipePropertiesString(const cl_pipe_properties* pProperties, const std::vector<cl_pipe_properties>& properties, bool include_brackets = true);

/// Convert mem flags to string
/// CL_MEM_READ_WRITE
/// CL_MEM_WRITE_ONLY
/// CL_MEM_READ_ONLY
/// CL_MEM_USE_HOST_PTR
/// CL_MEM_ALLOC_HOST_PTR
/// CL_MEM_COPY_HOST_PTR
/// CL_MEM_HOST_WRITE_ONLY
/// CL_MEM_HOST_READ_ONLY
/// CL_MEM_HOST_NO_ACCESS
/// \param flags mem flags
/// \return string representation of the input
std::string GetMemFlagsString(const cl_mem_flags flags);

/// Convert mem migration flags to string
/// CL_MIGRATE_MEM_OBJECT_HOST
/// CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED
/// \param flags mem flags
/// \return string representation of the input
std::string GetMemMigrationFlagsString(const cl_mem_migration_flags flags);

/// Convert map flags to string
/// CL_MAP_READ
/// CL_MAP_WRITE
/// CL_MAP_WRITE_INVALIDATE_REGION
/// \param flags map flags
/// \return string representation of the input
std::string GetMapFlagsString(const cl_map_flags flags);

/// Convert buffer create type to string
/// \param type buffer create type
/// \return string representation of the input
std::string GetBufferCreateString(const cl_buffer_create_type type);

/// Convert buffer create type to string
/// \param type buffer create type
/// \param info buffer create type data
/// \return string representation of the input
std::string GetBufferInfoString(const cl_buffer_create_type type, const void* info);

/// Convert cl_channel_order to string
/// CL_R
/// CL_A
/// CL_RG
/// CL_RA
/// CL_RGB
/// CL_RGBA
/// CL_BGRA
/// CL_ARGB
/// CL_INTENSITY
/// CL_LUMINANCE
/// CL_Rx
/// CL_RGx
/// CL_RGBx
/// \param order channel order
/// \return string representation of the input
std::string GetChannelOrderString(const cl_channel_order order);

/// Convert channel type to string
/// CL_SNORM_INT8
/// CL_SNORM_INT16
/// CL_UNORM_INT8
/// CL_UNORM_INT16
/// CL_UNORM_SHORT_565
/// CL_UNORM_SHORT_555
/// CL_UNORM_INT_101010
/// CL_SIGNED_INT8
/// CL_SIGNED_INT16
/// CL_SIGNED_INT32
/// CL_UNSIGNED_INT8
/// CL_UNSIGNED_INT16
/// CL_UNSIGNED_INT32
/// CL_HALF_FLOAT
/// CL_FLOAT
/// \param type channel type
/// \return string representation of the input
std::string GetChannelTypeString(const cl_channel_type type);

/// Convert image format to string
/// \param format image format array
/// \param num_entries size of array
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetImageFormatsString(const cl_image_format* format, size_t num_entries, bool include_brackets = true);

/// Convert addressing mode to string
/// CL_ADDRESS_NONE
/// CL_ADDRESS_CLAMP_TO_EDGE
/// CL_ADDRESS_CLAMP
/// CL_ADDRESS_REPEAT
/// CL_ADDRESS_MIRRORED_REPEAT
/// \param mode addressing mode
/// \return string representation of the input
std::string GetAddressingModeString(const cl_addressing_mode mode);

/// Convert filter mode to string
/// CL_FILTER_NEAREST
/// CL_FILTER_LINEAR
/// \param mode filter mode
/// \return string representation of the input
std::string GetFilterModeString(const cl_filter_mode mode);

/// Convert sampler info to string
/// CL_SAMPLER_REFERENCE_COUNT
/// CL_SAMPLER_CONTEXT
/// CL_SAMPLER_NORMALIZED_COORDS
/// CL_SAMPLER_ADDRESSING_MODE
/// CL_SAMPLER_FILTER_MODE
/// \param param_name sampler info enum
/// \return string representation of the input
std::string GetSamplerInfoString(const cl_sampler_info param_name);

/// Convert cl_sampler_info to string
/// \param param_name parameter name
/// \param param_value returned param value
/// \param ret_val the return value of API
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetSamplerInfoValueString(const cl_sampler_info param_name, const void* param_value, cl_int ret_val, bool include_brackets = true);

/// Convert cl_sampler_properties to string
/// \param pProperties the raw properties pointer
/// \param properties property vector
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetSamplerPropertiesString(const cl_sampler_properties* pProperties, std::vector<cl_sampler_properties> properties, bool include_brackets = true);

/// Convert device type enum to string
/// CL_DEVICE_TYPE_ALL
/// CL_DEVICE_TYPE_CPU
/// CL_DEVICE_TYPE_GPU
/// CL_DEVICE_TYPE_ACCELERATOR
/// CL_DEVICE_TYPE_CUSTOM
/// \param type device type enum
/// \return string representation of the input
std::string GetDeviceTypeString(const cl_device_type type);

/// Convert platform info to string
/// CL_PLATFORM_PROFILE
/// CL_PLATFORM_VERSION
/// CL_PLATFORM_NAME
/// CL_PLATFORM_VENDOR
/// CL_PLATFORM_EXTENSIONS
/// CL_PLATFORM_ICD_SUFFIX_KHR
/// \param param_name
/// \return string representation of the input
std::string GetPlatformInfoString(const cl_platform_info param_name);

/// Get platform info value string
/// \param param_value the value of the parameter
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetPlatformInfoValueString(const void* param_value, cl_int ret_value);

/// Convert device info to string
/// \param param_name device info
/// \return string representation of the input
std::string GetDeviceInfoString(const cl_device_info param_name);

/// Convert context info to string
/// \param param_name context info
/// \return string representation of the input
std::string GetContextInfoString(const cl_context_info param_name);

/// Convert context info to string
/// It could be the following info type
/// CL_CONTEXT_REFERENCE_COUNT
/// CL_CONTEXT_DEVICES
/// CL_CONTEXT_PROPERTIES
/// \param param_name info type
/// \param param_value_size data size
/// \param param_value pointer that points to the data
/// \param ret_value return value
/// \return string representation of the input
std::string GetContextInfoValueString(const cl_context_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value);

/// Convert command queue info to string
/// \param param_name command queue info enum
/// \return string representation of the input
std::string GetCommandQueueInfoString(const cl_command_queue_info param_name);

/// Convert cl_command_queue_info to string
/// \param param_name cl_command_queue_info
/// \param param_value returned param value
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetCommandQueueInfoValueString(const cl_command_queue_info param_name, const void* param_value, cl_int ret_val);

/// Convert program info to string
/// \param param_name program info enum
/// \return string representation of the input
std::string GetProgramInfoString(const cl_program_info param_name);

/// Convert cl_program_info to string
/// \param param_name parameter name
/// \param param_value_size data size
/// \param param_value returned param value
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetProgramInfoValueString(const cl_program_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_val);

/// Convert kernel info to string
/// \param param_name kernel info enum
/// \return string representation of the input
std::string GetKernelInfoString(const cl_kernel_info param_name);

/// Convert cl_kernel_info to string
/// \param param_name parameter name
/// \param param_value returned param value
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetKernelInfoValueString(const cl_kernel_info param_name, const void* param_value, cl_int ret_val);

/// Convert kernel arg info to string
/// \param param_name kernel arg info enum
/// \return string representation of the input
std::string GetKernelArgInfoString(const cl_kernel_arg_info param_name);

/// Convert cl_kernel_arg_info to string
/// \param param_name parameter name
/// \param param_value returned param value
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetKernelArgInfoValueString(const cl_kernel_arg_info param_name, const void* param_value, cl_int ret_val);

/// Convert kernel arg address qualifier to string
/// \param param_name kernel arg address qualifier enum
/// \return string representation of the input
std::string GetKernelArgAddressQualifierString(const cl_kernel_arg_address_qualifier param_name);

/// Convert kernel arg access qualifier to string
/// \param param_name kernel arg access qualifier enum
/// \return string representation of the input
std::string GetKernelArgAccessQualifierString(const cl_kernel_arg_access_qualifier param_name);

/// Convert kernel arg type qualifier to string
/// \param type kernel arg type qualifier enum
/// \return string representation of the input
std::string GetKernelArgTypeQualifierString(const cl_kernel_arg_type_qualifier type);

/// Convert cl_kernel_work_group_info to string
/// \param param_name cl_kernel_work_group_info enum
/// \return string representation of the input
std::string GetKernelWorkGroupInfoString(const cl_kernel_work_group_info param_name);

/// Convert cl_kernel_work_group_info to string
/// \param param_name cl_kernel_work_group_info
/// \param param_value returned param value
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetKernelWorkGroupInfoValueString(const cl_kernel_work_group_info param_name, const void* param_value, cl_int ret_value);

/// Convert cl_program_build_info to string
/// \param param_name cl_program_build_info
/// \return string representation of the input
std::string GetProgramBuildInfoString(const cl_program_build_info param_name);

/// Get cl_program_build_info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetProgramBuildInfoValueString(const cl_program_build_info param_name, const void* param_value, cl_int ret_value);

/// Convert cl_build_status to string
/// \param param_name cl_build_status
/// \return string representation of the input
std::string GetBuildStatusString(const cl_build_status param_name);

/// Convert cl_program_binary_type to string
/// \param param_name cl_program_binary_type
/// \return string representation of the input
std::string GetProgramBinaryTypeString(const cl_program_binary_type param_name);

/// Convert cl_event_info to string
/// \param param_name cl_event_info
/// \return string representation of the input
std::string GetEventInfoString(const cl_event_info param_name);

/// Convert cl_event_info value to string
/// \param param_name cl_event_info
/// \param param_value pointer to cl_event_info value
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetEventInfoValueString(const cl_event_info param_name, const void* param_value, cl_int ret_value);

/// Convert cl_profiling_info to string
/// \param param_name cl_profiling_info enum
/// \return string representation of the input
std::string GetProfilingInfoString(const cl_profiling_info param_name);

/// Convert cl_profiling_info value to string
/// \param param_name cl_profiling_info
/// \param param_value returned param value
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetProfilingInfoValueString(const cl_profiling_info param_name, const void* param_value, cl_int ret_value);

/// Convert CommandExecutionStatus to string
/// \param param_name CommandExecutionStatus code
/// \return string representation of the input
std::string GetExecutionStatusString(const cl_int param_name);

/// Return a quoted version of the specified string, unless the orginal source pointer is NULL
/// \param strInput the input string to quote
/// \param src the original source pointer
/// \return quoted verion of the input string
std::string GetQuotedString(const std::string& strInput, const char* src);

/// Convert add quotes to input string and optionally trucate it at most 60 chars
/// \param src input string
/// \param truncate flag indicating whether the output should be limited to 60 chars
/// \return string representation of the input
std::string GetStringString(const char* src, bool truncate = true);

/// Get program source string
/// \param strings input source string array
/// \param lengths string len array
/// \param count array size
/// \return string representation of the input
std::string GetProgramSourceString(const char** strings, const size_t* lengths, const cl_uint count);

/// Convert CL_FUNC_TYPE enum to string
/// \param type CL_FUNC_TYPE enum
/// \return string representation of the input
std::string GetCLAPINameString(const CL_FUNC_TYPE type);

/// Convert cl_command_type to string
/// \param type cl_command type
/// \return string representation of the input
std::string GetCommandTypeString(const cl_command_type type);

/// Return the event or NULL
/// \param event the Event returned by the API
/// \return string representation of the event, or "NULL"
std::string GetEventString(const cl_event event);

/// Convert event list to string
/// \param event_wait_list the event_wait_list parameter passed to the API
/// \param vecEvents list of events
/// \return string representation of the input
std::string GetEventListString(const cl_event* event_wait_list, const std::vector<cl_event>& vecEvents);

/// Convert value to string according to cl_device_info
/// \param param_name cl_device_info type
/// \param param_value_size data size
/// \param param_value pointer to value
/// \param ret_value the return value of API
/// \return string representation of the input
std::string GetDeviceInfoValueString(const cl_device_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value);

/// Return the size or NULL
/// \param size the size pointer returned by the API
/// \param sizeVal the dereferenced size pointer returned by the API
/// \return string representation of the size, or "NULL"
std::string GetSizeString(const size_t* size, size_t sizeVal);

/// Return the cl_uint or NULL
/// \param intPtr the cl_uint pointer returned by the API
/// \param intVal the dereferenced cl_uint pointer returned by the API
/// \return string representation of the cl_uint, or "NULL"
std::string GetIntString(const cl_uint* intPtr, cl_uint intVal);

/// Convert cl_device_partition_property_ext to string
/// \param prop the property to convert
/// \return string representation of the input
std::string GetPartitionPropertyString(const cl_device_partition_property prop);

/// Convert cl_device_partition_property_ext to affinity domain string
/// \param prop the property to convert
/// \return string representation of the input
std::string GetPartitionAffinityDomainString(const cl_device_affinity_domain prop);

/// Return a string representation of the cl_device_partition_property_ext list
/// \param properties the list of properties to display
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetPartitionPropertiesString(const std::vector<cl_device_partition_property>& properties, bool include_brackets = true);

/// Convert cl_device_partition_property_ext to string
/// \param prop the property to convert
/// \return string representation of the input
std::string GetPartitionPropertyExtString(const cl_device_partition_property_ext prop);

/// Convert cl_device_partition_property_ext to affinity domain string
/// \param prop the property to convert
/// \return string representation of the input
std::string GetPartitionAffinityDomainExtString(const cl_device_partition_property_ext prop);

/// Return a string representation of the cl_device_partition_property_ext list
/// \param properties the list of properties to display
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetPartitionPropertiesExtString(const std::vector<cl_device_partition_property_ext>& properties, bool include_brackets = true);

/// Convert mem info enum to string
/// GL_TEXTURE_2D
/// GL_TEXTURE_CUBE_MAP_POSITIVE_X
/// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
/// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
/// GL_TEXTURE_CUBE_MAP_NEGATIVE_X
/// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
/// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
/// GL_TEXTURE_RECTANGLE
/// \param texture_target cl_GLenum input
/// \return string representation of the input
std::string GetGLTextureTargetString(const cl_GLenum texture_target);

/// Convert mem info enum to string
/// CL_GL_TEXTURE_TARGET
/// CL_GL_MIPMAP_LEVEL
/// \param param_name mem_info
/// \return string representation of the input
std::string GetGLTextureInfoString(const cl_gl_texture_info param_name);

/// Get memory object info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_val the return value of API
/// \return string representation of the input
std::string GetGLTextureInfoValueString(const cl_gl_texture_info param_name, const void* param_value, cl_int ret_val);

/// Convert context info to string
/// \param param_name context info
/// \return string representation of the input
std::string GetGLContextInfoString(const cl_gl_context_info param_name);

/// Convert context info to string
/// CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR
/// CL_DEVICES_FOR_GL_CONTEXT_KHR
/// \param param_name info type
/// \param param_value_size data size
/// \param param_value pointer that points to the data
/// \param ret_value return value
/// \return string representation of the input
std::string GetGLContextInfoValueString(const cl_gl_context_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value);

/// Convert cl_gl_object_type to string
/// CL_GL_OBJECT_BUFFER
/// CL_GL_OBJECT_TEXTURE2D
/// CL_GL_OBJECT_TEXTURE3D
/// CL_GL_OBJECT_RENDERBUFFER
/// \param gl_object_type the pointer passed to the API
/// \param gl_object_typeVal the dereferenced value of a non-null pointer
/// \return string representation of the input
std::string GetGLObjectTypeString(cl_gl_object_type* gl_object_type, cl_gl_object_type gl_object_typeVal);

#ifdef _WIN32
    /// Convert cl_d3d10_device_source_khr to string
    /// CL_D3D10_DEVICE_KHR
    /// CL_D3D10_DXGI_ADAPTER_KHR
    /// \param d3d_device_source the cl_d3d10_device_source_khr object
    /// \return string representation of the input
    std::string GetD3D10DeviceSourceString(cl_d3d10_device_source_khr d3d_device_source);

    /// Convert cl_d3d10_device_set_khr to string
    /// CL_PREFERRED_DEVICES_FOR_D3D10_KHR
    /// CL_ALL_DEVICES_FOR_D3D10_KHR
    /// \param d3d_device_set the cl_d3d10_device_set_khr object
    /// \return string representation of the input
    std::string GetD3D10DeviceSetString(cl_d3d10_device_set_khr d3d_device_set);
#endif

/// Get string representing compiler options passed to clBuildProgram
/// \param strOptions the original options passed by the host program to clBuildProgram
/// \param options pointer value passed to clBuildProgram. If NULL, then "NULL" is displayed
/// \param strOverriddenOptions the actual options used by clBuildProgram. Can be different if one of two environment variables is set. If no overridden options are used, this is an empty string.
/// \param bOptionsAppended if overridden options are used, this flag indicates which env var contains the overrides
/// \param bIsLink flag indicating if the options are for a call to clLinkProgram
std::string GetBuildOptionsString(const std::string& strOptions, const char* options, const std::string& strOverriddenOptions, bool bOptionsAppended, bool bIsLink = false);

/// Get string representation of a cl_image_desc struct
/// \param imageDescriptor the input image descriptor
/// \return string representation of the input
std::string GetImageDescString(cl_image_desc* imageDescriptor);

/// Get string representation of the raw.type member of a cl_device_topology_amd struct
/// \param device_topology_type the input topology type
/// \return string representation of the input
std::string GetDeviceTopologyAMDTypeString(const cl_uint device_topology_type);

/// Get string representation of a cl_device_topology_amd struct
/// \param device_topology_amd the input device_topology_amd object
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetDeviceTopologyAMDString(cl_device_topology_amd* device_topology_amd, bool include_brackets);

/// Get string representation of a cl_kernel_exec_info enum
/// \param param_name the input cl_kernel_exec_info
/// \return string representation of the input
std::string GetKernelExecInfoString(const cl_kernel_exec_info param_name);

/// Get kernel exec info value string
/// \param param_name the name of the parameter
/// \param param_value the value of the parameter
/// \param ret_val the return value of API
/// \param param_value_size the param value data size
/// \return string representation of the input
std::string GetKernelExecInfoValueString(const cl_kernel_exec_info param_name, const void* param_value, cl_int ret_val, size_t param_value_size);

/// Convert pointer list to string
/// \param pointers the pointers parameter passed to the API
/// \param vecPointers list of pointers
/// \param include_brackets flag indicating if '[' and ']' should surround the string
/// \return string representation of the input
std::string GetPointerListString(void** pointers, const std::vector<void*>& vecPointers, bool include_brackets = true);

}

// @}

#endif //_CL_STRING_UTILS_H_
