//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLParameters.cpp
///
//==================================================================================

//------------------------------ apOpenCLParameters.cpp ------------------------------

// Standard C:
#include <stdarg.h>

// OS Definitions:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// Calculates the amount of bits we need to shirt right in order to convert one OpenCL Data type or another:
static unsigned int statIntToByteBitShift   = (sizeof(cl_uint) - sizeof(cl_uchar)) * GT_BITS_PER_BYTE;
static unsigned int statLongToByteBitShift   = (sizeof(cl_long) - sizeof(cl_uchar)) * GT_BITS_PER_BYTE;
// static unsigned int statULongToByteBitShift   = (sizeof(cl_ulong) - sizeof(cl_uchar)) * GT_BITS_PER_BYTE;
// static unsigned int statShortToByteBitShift = (sizeof(cl_ushort) - sizeof(cl_uchar)) * GT_BITS_PER_BYTE;


// ---------------------------- The use of stdarg ------------------------
//
//    a. We decided to use stdarg for logging the called function argument
//       values because of efficiency reasons.
//    b. In Microsoft implementation of stdarg, the C calling convention
//       force us to use:
//       - int for char, and short types
//       - double for float types
//       - pointer for array types
//       (See an appropriate comment at Microsoft stdarg.h file).
//
// ------------------------------------------------------------------------

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define _TEXT(x) L ## x
#endif

// The function that translate the cl enumeration to string contain many simple cases
// This macro spare the need to write those 3 line again and again:
#define AP_CL_ENUM_TOSTRING_CASE(enumExpression) \
    case enumExpression: \
    {   \
        valueString = _TEXT( # enumExpression ) ;\
        break;  \
    }

// ---------------------------------------------------------------------------
// Name:        apCLEnumValueToString
// Description: If the value is a know OpenCL enumerator, outputs its symbolic
//              name as a string.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/7/2015
// ---------------------------------------------------------------------------
bool apCLEnumValueToString(cl_uint value, gtString& valueString)
{
    bool retVal = true;

    switch (value)
    {
            AP_CL_ENUM_TOSTRING_CASE(CL_NONE);

            // cl_platform_info
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_PROFILE);
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_VERSION);
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_NAME);
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_VENDOR);
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_EXTENSIONS);

            // cl_device_info
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_VENDOR_ID);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_COMPUTE_UNITS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_WORK_GROUP_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_WORK_ITEM_SIZES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_CLOCK_FREQUENCY);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_ADDRESS_BITS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_READ_IMAGE_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_MEM_ALLOC_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE2D_MAX_WIDTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE2D_MAX_HEIGHT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE3D_MAX_WIDTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE3D_MAX_HEIGHT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE3D_MAX_DEPTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE_SUPPORT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_PARAMETER_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_SAMPLERS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MEM_BASE_ADDR_ALIGN);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SINGLE_FP_CONFIG);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_CONSTANT_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_LOCAL_MEM_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_LOCAL_MEM_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_ERROR_CORRECTION_SUPPORT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PROFILING_TIMER_RESOLUTION);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_ENDIAN_LITTLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_AVAILABLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_COMPILER_AVAILABLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_EXECUTION_CAPABILITIES);
            // AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_QUEUE_PROPERTIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_QUEUE_ON_HOST_PROPERTIES); // Replaced CL_DEVICE_QUEUE_PROPERTIES
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NAME);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_VENDOR);
            AP_CL_ENUM_TOSTRING_CASE(CL_DRIVER_VERSION);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PROFILE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_VERSION);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_EXTENSIONS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PLATFORM);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_DOUBLE_FP_CONFIG);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_HALF_FP_CONFIG); // Reserved, not yet officially in use.
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_HOST_UNIFIED_MEMORY);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_OPENCL_C_VERSION);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_LINKER_AVAILABLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_BUILT_IN_KERNELS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARENT_DEVICE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_MAX_SUB_DEVICES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_PROPERTIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_AFFINITY_DOMAIN);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PRINTF_BUFFER_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_ON_DEVICE_QUEUES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_ON_DEVICE_EVENTS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SVM_CAPABILITIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_PIPE_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PIPE_MAX_PACKET_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT);

            // cl_device_mem_cache_type
            // AP_CL_ENUM_TOSTRING_CASE(CL_NONE);
            // AP_CL_ENUM_TOSTRING_CASE(CL_READ_ONLY_CACHE);
            // AP_CL_ENUM_TOSTRING_CASE(CL_READ_WRITE_CACHE);

            // cl_device_local_mem_type
            // AP_CL_ENUM_TOSTRING_CASE(CL_LOCAL);
            // AP_CL_ENUM_TOSTRING_CASE(CL_GLOBAL);

            // cl_context_info
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_DEVICES);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_PROPERTIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_NUM_DEVICES);

            // cl_context_properties
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_PLATFORM);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_INTEROP_USER_SYNC);

            // cl_device_partition_property
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_EQUALLY);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_BY_COUNTS);
            // AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_BY_COUNTS_LIST_END); // Duplicate of CL_NONE
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN);

            // cl_command_queue_info
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_CONTEXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_DEVICE);
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_PROPERTIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_SIZE);

            // cl_channel_order
            AP_CL_ENUM_TOSTRING_CASE(CL_R);
            AP_CL_ENUM_TOSTRING_CASE(CL_A);
            AP_CL_ENUM_TOSTRING_CASE(CL_RG);
            AP_CL_ENUM_TOSTRING_CASE(CL_RA);
            AP_CL_ENUM_TOSTRING_CASE(CL_RGB);
            AP_CL_ENUM_TOSTRING_CASE(CL_RGBA);
            AP_CL_ENUM_TOSTRING_CASE(CL_BGRA);
            AP_CL_ENUM_TOSTRING_CASE(CL_ARGB);
            AP_CL_ENUM_TOSTRING_CASE(CL_INTENSITY);
            AP_CL_ENUM_TOSTRING_CASE(CL_LUMINANCE);
            AP_CL_ENUM_TOSTRING_CASE(CL_Rx);
            AP_CL_ENUM_TOSTRING_CASE(CL_RGx);
            AP_CL_ENUM_TOSTRING_CASE(CL_RGBx);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEPTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEPTH_STENCIL);
            AP_CL_ENUM_TOSTRING_CASE(CL_sRGB);
            AP_CL_ENUM_TOSTRING_CASE(CL_sRGBx);
            AP_CL_ENUM_TOSTRING_CASE(CL_sRGBA);
            AP_CL_ENUM_TOSTRING_CASE(CL_sBGRA);
            AP_CL_ENUM_TOSTRING_CASE(CL_ABGR);

            // cl_channel_type
            AP_CL_ENUM_TOSTRING_CASE(CL_SNORM_INT8);
            AP_CL_ENUM_TOSTRING_CASE(CL_SNORM_INT16);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_INT8);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_INT16);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_SHORT_565);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_SHORT_555);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_INT_101010);
            AP_CL_ENUM_TOSTRING_CASE(CL_SIGNED_INT8);
            AP_CL_ENUM_TOSTRING_CASE(CL_SIGNED_INT16);
            AP_CL_ENUM_TOSTRING_CASE(CL_SIGNED_INT32);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNSIGNED_INT8);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNSIGNED_INT16);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNSIGNED_INT32);
            AP_CL_ENUM_TOSTRING_CASE(CL_HALF_FLOAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_FLOAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_UNORM_INT24);

            // cl_mem_object_type
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE2D);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE3D);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE2D_ARRAY);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE1D);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE1D_ARRAY);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_IMAGE1D_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OBJECT_PIPE);

            // cl_mem_info
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_FLAGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_HOST_PTR);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_MAP_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_CONTEXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_ASSOCIATED_MEMOBJECT);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_OFFSET);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_USES_SVM_POINTER);

            // cl_image_info
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_FORMAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_ELEMENT_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_ROW_PITCH);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_SLICE_PITCH);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_WIDTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_HEIGHT);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_DEPTH);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_ARRAY_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_NUM_MIP_LEVELS);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_NUM_SAMPLES);

            // cl_pipe_info
            AP_CL_ENUM_TOSTRING_CASE(CL_PIPE_PACKET_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_PIPE_MAX_PACKETS);

            // cl_addressing_mode
            AP_CL_ENUM_TOSTRING_CASE(CL_ADDRESS_NONE);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADDRESS_CLAMP_TO_EDGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADDRESS_CLAMP);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADDRESS_REPEAT);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADDRESS_MIRRORED_REPEAT);

            // cl_filter_mode
            AP_CL_ENUM_TOSTRING_CASE(CL_FILTER_NEAREST);
            AP_CL_ENUM_TOSTRING_CASE(CL_FILTER_LINEAR);

            // cl_sampler_info
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_CONTEXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_NORMALIZED_COORDS);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_ADDRESSING_MODE);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_FILTER_MODE);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_MIP_FILTER_MODE);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_LOD_MIN);
            AP_CL_ENUM_TOSTRING_CASE(CL_SAMPLER_LOD_MAX);

            // cl_program_info
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_CONTEXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_NUM_DEVICES);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_DEVICES);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_SOURCE);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BINARY_SIZES);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BINARIES);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_NUM_KERNELS);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_KERNEL_NAMES);

            // cl_program_build_info
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BUILD_STATUS);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BUILD_OPTIONS);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BUILD_LOG);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BINARY_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE);

            // cl_build_status
            // AP_CL_ENUM_TOSTRING_CASE(CL_BUILD_SUCCESS);
            // AP_CL_ENUM_TOSTRING_CASE(CL_BUILD_NONE);
            // AP_CL_ENUM_TOSTRING_CASE(CL_BUILD_ERROR);
            // AP_CL_ENUM_TOSTRING_CASE(CL_BUILD_IN_PROGRESS);

            // cl_kernel_info
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_FUNCTION_NAME);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_NUM_ARGS);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_CONTEXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_PROGRAM);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ATTRIBUTES);

            // cl_kernel_arg_info:
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ADDRESS_QUALIFIER);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ACCESS_QUALIFIER);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_TYPE_NAME);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_TYPE_QUALIFIER);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_NAME);

            // cl_kernel_arg_address_qualifier:
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ADDRESS_GLOBAL);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ADDRESS_LOCAL);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ADDRESS_CONSTANT);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ADDRESS_PRIVATE);

            // cl_kernel_arg_access_qualifier:
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ACCESS_READ_ONLY);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ACCESS_WRITE_ONLY);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ACCESS_READ_WRITE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_ARG_ACCESS_NONE);

            // cl_kernel_work_group_info
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_WORK_GROUP_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_LOCAL_MEM_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_PRIVATE_MEM_SIZE);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_GLOBAL_WORK_SIZE);

            // cl_kernel_exec_info
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_EXEC_INFO_SVM_PTRS);
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM);

            // cl_event_info
            AP_CL_ENUM_TOSTRING_CASE(CL_EVENT_COMMAND_QUEUE);
            AP_CL_ENUM_TOSTRING_CASE(CL_EVENT_COMMAND_TYPE);
            AP_CL_ENUM_TOSTRING_CASE(CL_EVENT_REFERENCE_COUNT);
            AP_CL_ENUM_TOSTRING_CASE(CL_EVENT_COMMAND_EXECUTION_STATUS);
            AP_CL_ENUM_TOSTRING_CASE(CL_EVENT_CONTEXT);

            // cl_command_type
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_NDRANGE_KERNEL);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_TASK);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_NATIVE_KERNEL);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_READ_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_WRITE_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_COPY_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_READ_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_WRITE_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_COPY_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MAP_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MAP_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_UNMAP_MEM_OBJECT);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MARKER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_ACQUIRE_GL_OBJECTS);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_RELEASE_GL_OBJECTS);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_READ_BUFFER_RECT);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_WRITE_BUFFER_RECT);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_COPY_BUFFER_RECT);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_USER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_BARRIER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MIGRATE_MEM_OBJECTS);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_FILL_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_FILL_IMAGE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_SVM_FREE);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_SVM_MEMCPY);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_SVM_MEMFILL);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_SVM_MAP);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_SVM_UNMAP);

            // command execution status
            // AP_CL_ENUM_TOSTRING_CASE(CL_COMPLETE);
            // AP_CL_ENUM_TOSTRING_CASE(CL_RUNNING);
            // AP_CL_ENUM_TOSTRING_CASE(CL_SUBMITTED);
            // AP_CL_ENUM_TOSTRING_CASE(CL_QUEUED);

            // cl_buffer_create_type
            AP_CL_ENUM_TOSTRING_CASE(CL_BUFFER_CREATE_TYPE_REGION);

            // cl_profiling_info
            AP_CL_ENUM_TOSTRING_CASE(CL_PROFILING_COMMAND_QUEUED);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROFILING_COMMAND_SUBMIT);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROFILING_COMMAND_START);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROFILING_COMMAND_END);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROFILING_COMMAND_COMPLETE);

            // cl_gl
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_BUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE2D);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE3D);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_RENDERBUFFER);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE2D_ARRAY);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE1D);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE1D_ARRAY);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_OBJECT_TEXTURE_BUFFER);

            AP_CL_ENUM_TOSTRING_CASE(CL_GL_TEXTURE_TARGET);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_MIPMAP_LEVEL);
            AP_CL_ENUM_TOSTRING_CASE(CL_GL_NUM_SAMPLES);

            AP_CL_ENUM_TOSTRING_CASE(CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICES_FOR_GL_CONTEXT_KHR);

            AP_CL_ENUM_TOSTRING_CASE(CL_GL_CONTEXT_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_EGL_DISPLAY_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_GLX_DISPLAY_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_WGL_HDC_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CGL_SHAREGROUP_KHR);

            // cl_khr_gl_event:
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            // cl_khr_dx9_media_sharing:
            // cl_media_adapter_type_khr
            AP_CL_ENUM_TOSTRING_CASE(CL_ADAPTER_D3D9_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADAPTER_D3D9EX_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_ADAPTER_DXVA_KHR);

            // cl_media_adapter_set_khr
            AP_CL_ENUM_TOSTRING_CASE(CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR);

            // cl_context_info
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_ADAPTER_D3D9_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_ADAPTER_D3D9EX_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_ADAPTER_DXVA_KHR);

            // cl_mem_info
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_DX9_MEDIA_ADAPTER_TYPE_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_DX9_MEDIA_SURFACE_INFO_KHR);

            // cl_image_info
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_DX9_MEDIA_PLANE_KHR);

            // cl_command_type
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR);

            // cl_khr_d3d10_sharing:
            // cl_d3d10_device_source_nv
            AP_CL_ENUM_TOSTRING_CASE(CL_D3D10_DEVICE_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_D3D10_DXGI_ADAPTER_KHR);

            // cl_d3d10_device_set_nv
            AP_CL_ENUM_TOSTRING_CASE(CL_PREFERRED_DEVICES_FOR_D3D10_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_ALL_DEVICES_FOR_D3D10_KHR);

            // cl_context_info
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_D3D10_DEVICE_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_D3D10_PREFER_SHARED_RESOURCES_KHR);

            // cl_mem_info
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_D3D10_RESOURCE_KHR);

            // cl_image_info
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_D3D10_SUBRESOURCE_KHR);

            // cl_command_type
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR);

            // cl_khr_d3d11_sharing:
            // cl_d3d11_device_source
            AP_CL_ENUM_TOSTRING_CASE(CL_D3D11_DEVICE_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_D3D11_DXGI_ADAPTER_KHR);

            // cl_d3d11_device_set
            AP_CL_ENUM_TOSTRING_CASE(CL_PREFERRED_DEVICES_FOR_D3D11_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_ALL_DEVICES_FOR_D3D11_KHR);

            // cl_context_info
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_D3D11_DEVICE_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_D3D11_PREFER_SHARED_RESOURCES_KHR);

            // cl_mem_info
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_D3D11_RESOURCE_KHR);

            // cl_image_info
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_D3D11_SUBRESOURCE_KHR);

            // cl_command_type
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            // CLADDITIONS_H:

            // CLEXT_H:
            // cl_khr_fp64:
            // CL_DEVICE_DOUBLE_FP_CONFIG already handled above

            // cl_ext_migrate_memobject:
            // AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MIGRATE_MEM_OBJECT_EXT); // = CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD causing a conflict, and replaced by CL_COMMAND_MIGRATE_MEM_OBJECTS

#if (AMDT_BUILD_TARGET == AMDT_DEBUG_BUILD) || (AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS)
            // AMD internal extensions, hide these value from NDA / public users:

            // cl_amd_command_intercept:
            // AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_COMMAND_INTERCEPT_CALLBACK_AMD); // Moved to public cl_ext.h file as of APP SDK 3.0.

            // cl_amd_object_metadata:
            // AP_CL_ENUM_TOSTRING_CASE(CL_INVALID_OBJECT_AMD); // Handled in EXT version
            // AP_CL_ENUM_TOSTRING_CASE(CL_INVALID_KEY_AMD); // Handled in EXT version
            // AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_MAX_KEYS_AMD); // Handled in EXT version
#endif

            // cl_ext_object_metadata
            AP_CL_ENUM_TOSTRING_CASE(CL_INVALID_OBJECT_EXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_INVALID_KEY_EXT);
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_MAX_KEYS_EXT);

            // cl_khr_terminate_context
            // AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_TERMINATE_CAPABILITY_KHR); // Equal to CL_GL_OBJECT_TEXTURE1D
            // AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_TERMINATE_KHR);           // Equal to CL_GL_OBJECT_TEXTURE1D_ARRAY

            // cl_khr_sub_group_info
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR); // = CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR
            AP_CL_ENUM_TOSTRING_CASE(CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE_KHR);    // = CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE_KHR

            // cl_khr_icd
            AP_CL_ENUM_TOSTRING_CASE(CL_PLATFORM_ICD_SUFFIX_KHR);

            // cl_khr_initalize_memory extension
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_MEMORY_INITIALIZE_KHR);

            // cl_khr_terminate_context extension
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_TERMINATE_CAPABILITY_KHR);
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_TERMINATE_KHR);

            // cl_khr_spir
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SPIR_VERSIONS);
            AP_CL_ENUM_TOSTRING_CASE(CL_PROGRAM_BINARY_TYPE_INTERMEDIATE);

            // cl_nv_device_attribute_query
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_REGISTERS_PER_BLOCK_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_WARP_SIZE_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GPU_OVERLAP_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_INTEGRATED_MEMORY_NV);

            // cl_amd_device_memory_flags - cl_ext_device_memory_flags?
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT);

            // cl_amd_device_attribute_query
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PROFILING_TIMER_OFFSET_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_TOPOLOGY_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_BOARD_NAME_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_FREE_MEMORY_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SIMD_WIDTH_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_WAVEFRONT_WIDTH_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_LOCAL_MEM_BANKS_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GFXIP_MAJOR_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_GFXIP_MINOR_AMD);

            // cl_amd_command_intercept
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_COMMAND_INTERCEPT_CALLBACK_AMD);

            // cl_amd_command_queue_info
            AP_CL_ENUM_TOSTRING_CASE(CL_QUEUE_THREAD_HANDLE_AMD);

            // cl_amd_offline_devices
            AP_CL_ENUM_TOSTRING_CASE(CL_CONTEXT_OFFLINE_DEVICES_AMD);

            // cl_arm_printf
            AP_CL_ENUM_TOSTRING_CASE(CL_PRINTF_CALLBACK_ARM);
            AP_CL_ENUM_TOSTRING_CASE(CL_PRINTF_BUFFERSIZE_ARM);

            // cl_qcom_ext_host_ptr
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_EXT_MEM_PADDING_IN_BYTES_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_DEVICE_PAGE_SIZE_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_ROW_ALIGNMENT_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_IMAGE_SLICE_ALIGNMENT_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_HOST_UNCACHED_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_HOST_WRITEBACK_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_HOST_WRITETHROUGH_QCOM);
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_HOST_WRITE_COMBINING_QCOM);

            // cl_qcom_ion_host_ptr
            AP_CL_ENUM_TOSTRING_CASE(CL_MEM_ION_HOST_PTR_QCOM);

            // cl_amd_bus_addressable_memory
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_WAIT_SIGNAL_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_WRITE_SIGNAL_AMD);
            AP_CL_ENUM_TOSTRING_CASE(CL_COMMAND_MAKE_BUFFERS_RESIDENT_AMD);

        default:
            valueString.makeEmpty().appendFormattedString(L"%#06x (Unknown OpenCL enumerator)");
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apAddCLBitfieldBitToValueString
// Description:
//   Is called repeatedly by valueAsString() for every known OpenCL memory bit.
//   Adds the bit to the output valueString if the bit appears in the bit-mask that we represent.
// Arguments: bit - The input memory flag bit.
//            bitAsString - The input memory flag bit as a string.
//            valueString - The output value string.
//            wasBitEncountered - Input and output. Will get true iff we already encountered
//                                a bit that appear in the bit-mask that we represent.
//            pipeAsSeparator - true = separate with '|'
//                              false, separate with ','
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
void apAddCLBitfieldBitToValueString(cl_bitfield value, cl_bitfield bit, const wchar_t* bitAsString, gtString& valueString, bool& wasBitEncountered, bool pipeAsSeparator)
{
    // If the bit appears in the bit-mask that we represent:
    if (value & bit)
    {
        // If we already encountered a bit that appear in the bit-mask that we represent:
        if (wasBitEncountered)
        {
            valueString += pipeAsSeparator ? L" | " : L", ";
        }

        // Mark that the input bit appears in the bit-mask that we represent:
        wasBitEncountered = true;

        // Add the bit's string to the output value string:
        valueString += bitAsString;
    }
}

// -----------------------------   apCLuintParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLuintParameter::type() const
{
    return OS_TOBJ_ID_CL_UINT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLuintParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLuintParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_uint)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLuintParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_uint);
}


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void apCLuintParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_uint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gtSizeType apCLuintParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_uint);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLuintParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void apCLuintParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLuintParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLuintParameter:
        apCLuintParameter* pParam  = (apCLuintParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLuintParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statIntToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLuintParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLuintParameter::setValueFromDouble(double value)
{
    _value = (cl_uint)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLuintParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLuintParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apCLintParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLintParameter::type() const
{
    return OS_TOBJ_ID_CL_INT_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLintParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLintParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (cl_int)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLintParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_int);
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apCLintParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_int*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
gtSizeType apCLintParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_int);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLintParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apCLintParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%d", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLintParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLintParameter:
        apCLintParameter* pParam  = (apCLintParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLintParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLintParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statIntToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLintParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLintParameter::setValueFromDouble(double value)
{
    _value = (cl_int)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLintParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLintParameter::valueAsDouble()
{
    return (double)_value;
}


// -----------------------------   apCLucharParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apCLucharParameter::type() const
{
    return OS_TOBJ_ID_CL_UCHAR_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLucharParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLucharParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_uchar)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLucharParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // To avoid compiler warnings we have to use different types for MSVC and gcc.
    // For MSVC we use cl_uchar otherwise MSVC warns about mismatching types
    // For gcc we use int otherwise gcc warns about char begin promoted to int when passed to '...'
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#define COMPILER_APPROVED_TYPE int
#else  // Windows
#define COMPILER_APPROVED_TYPE cl_uchar
#endif

    COMPILER_APPROVED_TYPE value = va_arg(pArgumentList, COMPILER_APPROVED_TYPE);
    _value = (cl_uchar)value;
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLucharParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_uchar*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
gtSizeType apCLucharParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_uchar);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLucharParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_2_CHAR_FORMAT, (gtUByte)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLucharParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLucharParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLucharParameter:
        apCLucharParameter* pParam  = (apCLucharParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLucharParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return _value;
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLucharParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLucharParameter::setValueFromDouble(double value)
{
    _value = (cl_uchar)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLucharParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLucharParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apCLcharParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apCLcharParameter::type() const
{
    return OS_TOBJ_ID_CL_CHAR_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLcharParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLcharParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (cl_char)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLcharParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // To avoid compiler warnings we have to use different types for MSVC and gcc.
    // For MSVC we use cl_char otherwise MSVC warns about mismatching types
    // For gcc we use int otherwise gcc warns about char begin promoted to int when passed to '...'
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#define COMPILER_APPROVED_TYPE int
#else  // Windows
#define COMPILER_APPROVED_TYPE cl_char
#endif

    COMPILER_APPROVED_TYPE value = va_arg(pArgumentList, COMPILER_APPROVED_TYPE);
    _value = (cl_char)value;
}


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLcharParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_char*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
gtSizeType apCLcharParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_char);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLcharParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_2_CHAR_FORMAT, (gtUByte)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLcharParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%d", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLcharParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLcharParameter:
        apCLcharParameter* pParam  = (apCLcharParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLcharParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return _value;
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLcharParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLcharParameter::setValueFromDouble(double value)
{
    _value = (cl_char)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLcharParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLcharParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apCLulongParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apCLulongParameter::type() const
{
    return OS_TOBJ_ID_CL_ULONG_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLulongParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLulongParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_ulong)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLulongParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_ulong);
}


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLulongParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_ulong*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
gtSizeType apCLulongParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_ulong);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLulongParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, (gtUInt64)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLulongParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%lu", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLulongParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLulongParameter:
        apCLulongParameter* pParam  = (apCLulongParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLulongParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statLongToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLulongParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLulongParameter::setValueFromDouble(double value)
{
    _value = (cl_long)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLulongParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLulongParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apCLlongParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apCLlongParameter::type() const
{
    return OS_TOBJ_ID_CL_LONG_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLlongParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLlongParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 valueAsInt32 = 0;
    ipcChannel >> valueAsInt32;
    _value = (cl_long)valueAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLlongParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_long);
}


// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLlongParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_long*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
gtSizeType apCLlongParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_long);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLlongParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, (gtInt64)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLlongParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%ld", _value);
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool apCLlongParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLlongParameter:
        apCLlongParameter* pParam  = (apCLlongParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::asPixelValue
// Description: Converts the value to a pixel value
// Return Val:  Pixel value mapped to [0..255]
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
char apCLlongParameter::asPixelValue() const
{
    // Should we use default multiplier?
    if (_isDefaultMulitiplier)
    {
        return char(_value >> statLongToByteBitShift);
    }
    else
    {
        // Use manual value multiplier
        return char((double)_value * _valueMultiplier);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::shiftAndMaskBits
// Description: Shifts and masks the bits of the Parameter value
// Arguments:   bitsMask - The bit mask to mask the value with
//              amountOfBitsToShiftRight - The amount of bits to shift left
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLlongParameter::shiftAndMaskBits(unsigned int bitsMask, int amountOfBitsToShiftRight)
{
    // Shift the value to the relevant position
    _value = _value >> amountOfBitsToShiftRight;

    // Mask value with bit mask
    _value &= bitsMask;
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::setValueFromDouble
// Description: Set my value from a Double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void apCLlongParameter::setValueFromDouble(double value)
{
    _value = (cl_long)value;
}

// ---------------------------------------------------------------------------
// Name:        apCLlongParameter::valueAsDouble
// Description: Return my value as a double
// Author:  AMD Developer Tools Team
// Date:        27/3/2011
// ---------------------------------------------------------------------------
double apCLlongParameter::valueAsDouble()
{
    return (double)_value;
}

// -----------------------------   apCLBoolParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLBoolParameter::type() const
{
    return OS_TOBJ_ID_CL_BOOL_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLBoolParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLBoolParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLBoolParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_bool);
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apCLBoolParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_bool*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
gtSizeType apCLBoolParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_bool);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void apCLBoolParameter::valueAsString(gtString& valueString) const
{
    if (_value == 0)
    {
        valueString = AP_STR_FALSE;
    }
    else
    {
        valueString = AP_STR_TRUE;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLBoolParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool apCLBoolParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLBoolParameter:
        apCLBoolParameter* pParam  = (apCLBoolParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}



// -----------------------------   apCLMemFlags ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::apCLMemFlags
// Description: Constructor
// Arguments: memoryFlags - The OpenCL memory flags bitfield.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLMemFlags::apCLMemFlags(cl_mem_flags memoryFlags)
    : _value(memoryFlags)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::~apCLMemFlags
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLMemFlags::~apCLMemFlags()
{
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMemFlags::type() const
{
    return OS_TOBJ_ID_CL_MEM_FLAGS_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLMemFlags::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLMemFlags::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (cl_mem_flags)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLMemFlags::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_mem_flags);
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void apCLMemFlags::readValueFromPointer(void* pValue)
{
    _value = *((cl_uint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gtSizeType apCLMemFlags::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_mem_flags);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLMemFlags::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLMemFlags:
        apCLMemFlags* pParam  = (apCLMemFlags*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::getSpecialValue
// Description: Gets the special value string for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLMemFlags::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLMemFlags::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemFlags::getBitName
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool apCLMemFlags::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_MEM_READ_WRITE";
            break;

        case 1:
            o_string = L"CL_MEM_WRITE_ONLY";
            break;

        case 2:
            o_string = L"CL_MEM_READ_ONLY";
            break;

        case 3:
            o_string = L"CL_MEM_USE_HOST_PTR";
            break;

        case 4:
            o_string = L"CL_MEM_ALLOC_HOST_PTR";
            break;

        case 5:
            o_string = L"CL_MEM_COPY_HOST_PTR";
            break;

        case 6:
            o_string = L"CL_MEM_USE_PERSISTENT_MEM_AMD";
            break;

        case 7:
            o_string = L"CL_MEM_HOST_WRITE_ONLY";
            break;

        case 8:
            o_string = L"CL_MEM_HOST_READ_ONLY";
            break;

        case 9:
            o_string = L"CL_MEM_HOST_NO_ACCESS";
            break;

        case 29:
            o_string = L"CL_MEM_EXT_HOST_PTR_QCOM";
            break;

        case 30:
            o_string = L"CL_MEM_BUS_ADDRESSABLE_AMD";
            break;

        case 31:
            o_string = L"CL_MEM_EXTERNAL_PHYSICAL_AMD";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// ----------------------------- apCLSVMMemFlagsParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFlagsParameter::apCLSVMMemFlagsParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apCLSVMMemFlagsParameter::apCLSVMMemFlagsParameter(cl_svm_mem_flags memoryFlags)
    : apCLMemFlags((cl_mem_flags)memoryFlags)
{

}
// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFlagsParameter::~apCLSVMMemFlagsParameter
// Description: Destrcutor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apCLSVMMemFlagsParameter::~apCLSVMMemFlagsParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFlagsParameter::type
// Description:
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMMemFlagsParameter::type() const
{
    return OS_TOBJ_ID_CL_SVM_MEM_FLAGS_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFlagsParameter::getBitName
// Description: Gets the bit name, specializing for cl_svm_mem_flags over cl_mem_flags
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLSVMMemFlagsParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 10:
            o_string = L"CL_MEM_SVM_FINE_GRAIN_BUFFER";
            break;

        case 11:
            o_string = L"CL_MEM_SVM_ATOMICS";
            break;

        default:
            retVal = apCLMemFlags::getBitName(bitIndex, o_string);
            break;
    }

    return retVal;
}

// -----------------------------   apCLGLObjectTypeParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLGLObjectTypeParameter::type() const
{
    return OS_TOBJ_ID_CL_GL_OBJECT_TYPE_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLObjectTypeParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLObjectTypeParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_uint)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLGLObjectTypeParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_gl_object_type);
}


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
void apCLGLObjectTypeParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_uint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
gtSizeType apCLGLObjectTypeParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_uint);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLGLObjectTypeParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
void apCLGLObjectTypeParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLGLObjectTypeParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLObjectTypeParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLGLObjectTypeParameter:
        apCLGLObjectTypeParameter* pParam  = (apCLGLObjectTypeParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apCLGLTextureInfoParameter ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLGLTextureInfoParameter::type() const
{
    return OS_TOBJ_ID_CL_GL_TEXTURE_INFO_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLTextureInfoParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLTextureInfoParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_uint)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLGLTextureInfoParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_gl_texture_info);
}


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
void apCLGLTextureInfoParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_uint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
gtSizeType apCLGLTextureInfoParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_uint);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLGLTextureInfoParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, (gtUInt32)_value);
}

// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
void apCLGLTextureInfoParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u", _value);
}


// ---------------------------------------------------------------------------
// Name:        apCLGLTextureInfoParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/11/2009
// ---------------------------------------------------------------------------
bool apCLGLTextureInfoParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLGLTextureInfoParameter:
        apCLGLTextureInfoParameter* pParam  = (apCLGLTextureInfoParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// -----------------------------   apCLMultiStringParameter ------------------------------



// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::apCLMultiStringParameter
// Description: Constructor
// Arguments:   count - The amount of input strings.
//              strings - The array of strings.
//              lengths - An array containing the string lengths.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
apCLMultiStringParameter::apCLMultiStringParameter(cl_uint count, const char** strings, const size_t* lengths)
{
    setCLStrings(count, strings, lengths);
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMultiStringParameter::type() const
{
    return OS_TOBJ_ID_CL_MULTI_STRING_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
bool apCLMultiStringParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the amount of strings:
    gtInt64 amountOfStrings = (gtInt64)_value.size();
    ipcChannel << amountOfStrings;

    // Write the strings:
    for (int i = 0; i < amountOfStrings; i++)
    {
        ipcChannel << _value[i];
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
bool apCLMultiStringParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    // Clear the strings vector:
    _value.clear();

    // Read the amount of strings:
    gtInt64 amountOfStrings = 0;
    ipcChannel >> amountOfStrings;

    // Read the strings:
    for (int i = 0; i < amountOfStrings; i++)
    {
        gtASCIIString currentString;
        ipcChannel >> currentString;
        _value.push_back(currentString);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLMultiStringParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    // Get the amount of strings:
    cl_uint count = va_arg(pArgumentList , cl_uint);

    // Get the strings array:
    const char** strings = (const char**)(va_arg(pArgumentList , void**));

    // Get the string lengths:
    const size_t* lengths = va_arg(pArgumentList , size_t*);

    // Copy the strings into this class:
    setCLStrings(count, strings, lengths);
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
void apCLMultiStringParameter::readValueFromPointer(void* pValue)
{
    (void)(pValue); // unused
    // Should not be called !
    GT_ASSERT(0);
    _value.clear();
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
gtSizeType apCLMultiStringParameter::sizeofData()
{
    // This function should not be called !!!
    GT_ASSERT(0);

    static gtSizeType sizeOfChar = sizeof(char);
    return sizeOfChar;
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// Implementation notes:
//  a. We concatenate all the strings into one big string.
//  b. After every original string, except the last, we add a new line
//  c. We replace every carriage return '\r' with a new line '\n'
// ---------------------------------------------------------------------------
void apCLMultiStringParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    int amountOfStrings = (int)_value.size();
    int lastStringIndex = amountOfStrings - 1;

    for (int i = 0; i < amountOfStrings; i++)
    {
        gtString currentString;
        currentString.fromASCIIString(_value[i].asCharArray());
        valueString += currentString;

        int currentLength = valueString.length();
        int lastCharPos = currentLength - 1;

        // If this is not the last string:
        if (i != lastStringIndex)
        {
            // If the string does not end with a new line / carriage return - add a new line:
            wchar_t lastChar = '\0';

            if (lastCharPos >= 0)
            {
                lastChar = valueString[lastCharPos];
            }

            if (!((lastChar == '\r') || (lastChar == '\n')))
            {
                valueString += '\n';
            }
        }
    }

    // Iterate the resulting string:
    int valueStringLength = valueString.length();
    int lastCharPos = valueStringLength - 1;

    for (int i = 0; i < valueStringLength; i++)
    {
        // If the current char is a carriage return:
        if (valueString[i] == '\r')
        {
            // If we have a sequence of '\r\n' - we will replace the '\r' with a space,
            // otherwise - we will replace it with a new line:
            if ((i != lastCharPos) && (valueString[i + 1] == '\n'))
            {
                valueString[i] = ' ';
            }
            else
            {
                valueString[i] = '\n';
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// Implementation notes:
//  a. We concatenate all the strings into one big string.
//  b. After every original string, except the last, we add a new line
//  c. We replace every carriage return '\r' with a new line '\n'
// ---------------------------------------------------------------------------
void apCLMultiStringParameter::valueAsString(gtASCIIString& valueString) const
{
    valueString.makeEmpty();

    int amountOfStrings = (int)_value.size();
    int lastStringIndex = amountOfStrings - 1;

    for (int i = 0; i < amountOfStrings; i++)
    {
        gtASCIIString currentString = _value[i].asCharArray();
        valueString += currentString;

        int currentLength = valueString.length();
        int lastCharPos = currentLength - 1;

        // If this is not the last string:
        if (i != lastStringIndex)
        {
            // If the string does not end with a new line / carriage return - add a new line:
            char lastChar = '\0';

            if (lastCharPos >= 0)
            {
                lastChar = valueString[lastCharPos];
            }

            if (!((lastChar == '\r') || (lastChar == '\n')))
            {
                valueString += '\n';
            }
        }
    }

    // Iterate the resulting string:
    // Replace \r\n to \n:
    valueString.replace("\r\n", "\n", true);

    // Replace \r to \n:
    valueString.replace("\r", "\n", true);
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
bool apCLMultiStringParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLHandleParameter:
        apCLMultiStringParameter* pParam  = (apCLMultiStringParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLMultiStringParameter::setCLStrings
// Description: Sets my contained strings.
// Arguments:   count - The amount of input strings.
//              strings - The array of strings.
//              lengths - An array containing the string lengths.
// Author:  AMD Developer Tools Team
// Date:        16/11/2009
// ---------------------------------------------------------------------------
void apCLMultiStringParameter::setCLStrings(cl_uint count, const char** strings, const size_t* lengths)
{
    // Clear the strings vector:
    _value.clear();

    bool haveLengths = (lengths != NULL);

    // Add the input strings into the vector:
    for (cl_uint i = 0; i < count; i++)
    {
        gtASCIIString currentString;

        // See if we have the current string's length:
        int currentStringLength = -1;

        if (haveLengths)
        {
            currentStringLength = (int)lengths[i];
        }

        // A string length of 0 or less is "use null terminator:
        const char* curStrCharArray = strings[i];

        if (nullptr != curStrCharArray)
        {
            if (currentStringLength < 1)
            {
                currentString.append(curStrCharArray);
            }
            else // 1 <= currentStringLength
            {
                // Append the current char* to a string:
                currentString.append(curStrCharArray, currentStringLength);
            }
        }
        else
        {
            GT_ASSERT(currentStringLength < 1);
        }

        _value.push_back(currentString);
    }
}


// -----------------------------   apCLHandleParameter ------------------------------
apCLHandleParameter::apCLHandleParameter(oaCLHandle ptrValue, osTransferableObjectType pointedObjectType):
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    _is64BitPointer(false),
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    _is64BitPointer(true),
#else
#error Unknown address space size!
#endif
    _ptrValue(ptrValue), _pointedObjectType(pointedObjectType)
{

}
// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::id
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLHandleParameter::type() const
{
    return OS_TOBJ_ID_CL_HANDLE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool apCLHandleParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _is64BitPointer;
    ipcChannel << (gtUInt64)_ptrValue;
    ipcChannel << (gtInt32)_pointedObjectType;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool apCLHandleParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _is64BitPointer;
    gtUInt64 ptrValueAsUInt64 = 0;
    ipcChannel >> ptrValueAsUInt64;
    _ptrValue = (oaCLHandle)ptrValueAsUInt64;
    gtInt32 pointedObjectTypeAsInt32 = 0;
    ipcChannel >> pointedObjectTypeAsInt32;
    _pointedObjectType = (osTransferableObjectType)pointedObjectTypeAsInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void apCLHandleParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _ptrValue = (oaCLHandle)va_arg(pArgumentList , void*);

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void apCLHandleParameter::readValueFromPointer(void* pValue)
{
    _ptrValue = (oaCLHandle) * ((void**)(pValue));

    // We cannot know the data type. We initialize it to be int:
    _pointedObjectType = OS_TOBJ_ID_INT_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
gtSizeType apCLHandleParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(void*);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void apCLHandleParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();

    if (_is64BitPointer)
    {
        valueString.appendFormattedString(GT_64_BIT_POINTER_FORMAT_UPPERCASE, _ptrValue);
    }
    else
    {
        gtUInt32 ptrPtrValueAsUInt32 = (gtUInt32)_ptrValue;
        valueString.appendFormattedString(GT_32_BIT_POINTER_FORMAT_UPPERCASE, ptrPtrValueAsUInt32);
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLHandleParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool apCLHandleParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apCLHandleParameter:
        apCLHandleParameter* pPointerParameter = (apCLHandleParameter*)(&other);
        GT_IF_WITH_ASSERT(pPointerParameter != NULL)
        {
            retVal = (pPointerParameter->_ptrValue == _ptrValue);
        }
    }

    return retVal;
}

// -----------------------------  apCLDeviceTypeParameter  ------------------------------


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::setValue
// Description: Sets my value, and asserts the fact it was not already set.
//              This should only be used by apCLDevice, and only once when initializing
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceTypeParameter::setValue(const cl_device_type& newValue)
{
    GT_ASSERT(_value == 0);
    _value = newValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDeviceTypeParameter::type() const
{
    return OS_TOBJ_ID_CL_DEVICE_TYPE_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLDeviceTypeParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLDeviceTypeParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_device_type)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLDeviceTypeParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_device_type);
}


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void apCLDeviceTypeParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_device_type*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
gtSizeType apCLDeviceTypeParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_device_type);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLDeviceTypeParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLDeviceTypeParameter* pParam  = (apCLDeviceTypeParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE and CL_DEVICE_TYPE_ALL
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceTypeParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }
    else if (CL_DEVICE_TYPE_ALL == _value)
    {
        retVal = true;
        o_string = L"CL_DEVICE_TYPE_ALL";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLDeviceTypeParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLDeviceTypeParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_DEVICE_TYPE_DEFAULT";
            break;

        case 1:
            o_string = L"CL_DEVICE_TYPE_CPU";
            break;

        case 2:
            o_string = L"CL_DEVICE_TYPE_GPU";
            break;

        case 3:
            o_string = L"CL_DEVICE_TYPE_ACCELERATOR";
            break;

        case 4:
            o_string = L"CL_DEVICE_TYPE_CUSTOM";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ----------------------------- apCLDeviceExecutionCapabilitiesParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::setValue
// Description: Sets my value, and asserts the fact it was not already set.
//              This should only be used by apCLDevice, and only once when initializing
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceExecutionCapabilitiesParameter::setValue(const cl_device_type& newValue)
{
    GT_ASSERT(_value == 0);
    _value = newValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDeviceExecutionCapabilitiesParameter::type() const
{
    return OS_TOBJ_ID_CL_DEVICE_EXECUTION_CAPABILITIES_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceExecutionCapabilitiesParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceExecutionCapabilitiesParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_device_exec_capabilities)valueAsUInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLDeviceExecutionCapabilitiesParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_device_exec_capabilities);
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceExecutionCapabilitiesParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_device_exec_capabilities*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
gtSizeType apCLDeviceExecutionCapabilitiesParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_device_exec_capabilities);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceExecutionCapabilitiesParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLDeviceExecutionCapabilitiesParameter* pParam = (apCLDeviceExecutionCapabilitiesParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceExecutionCapabilitiesParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLDeviceExecutionCapabilitiesParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceExecutionCapabilitiesParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceExecutionCapabilitiesParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_EXEC_KERNEL";
            break;

        case 1:
            o_string = L"CL_EXEC_NATIVE_KERNEL";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ----------------------------- apCLDeviceFloatingPointConfigParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::setValue
// Description: Sets my value, and asserts the fact it was not already set.
//              This should only be used by apCLDevice, and only once when initializing
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceFloatingPointConfigParameter::setValue(const cl_device_fp_config& newValue)
{
    GT_ASSERT(_value == 0);
    _value = newValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDeviceFloatingPointConfigParameter::type() const
{
    return OS_TOBJ_ID_CL_DEVICE_FLOATING_POINT_CONFIG_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceFloatingPointConfigParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceFloatingPointConfigParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_device_fp_config)valueAsUInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLDeviceFloatingPointConfigParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_device_fp_config);
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceFloatingPointConfigParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_device_fp_config*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
gtSizeType apCLDeviceFloatingPointConfigParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_device_fp_config);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::compareToOther
// Description: Compares this with other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceFloatingPointConfigParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLDeviceFloatingPointConfigParameter* pParam = (apCLDeviceFloatingPointConfigParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceFloatingPointConfigParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLDeviceFloatingPointConfigParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceFloatingPointConfigParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        25/3/2010
// ---------------------------------------------------------------------------
bool apCLDeviceFloatingPointConfigParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_FP_DENORM";
            break;

        case 1:
            o_string = L"CL_FP_INF_NAN";
            break;

        case 2:
            o_string = L"CL_FP_ROUND_TO_NEAREST";
            break;

        case 3:
            o_string = L"CL_FP_ROUND_TO_ZERO";
            break;

        case 4:
            o_string = L"CL_FP_ROUND_TO_INF";
            break;

        case 5:
            o_string = L"CL_FP_FMA";
            break;

        case 6:
            o_string = L"CL_FP_SOFT_FLOAT";
            break;

        case 7:
            o_string = L"CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// -----------------------------   apCLCommandQueuePropertiesParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCommandQueuePropertiesParameter::type() const
{
    return OS_TOBJ_ID_CL_COMMAND_QUEUE_PROPERTIES_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertiesParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertiesParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_command_queue_properties)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLCommandQueuePropertiesParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_command_queue_properties);
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void apCLCommandQueuePropertiesParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_command_queue_properties*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
gtSizeType apCLCommandQueuePropertiesParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_command_queue_properties);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertiesParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLCommandQueuePropertiesParameter* pParam  = (apCLCommandQueuePropertiesParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertiesParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLCommandQueuePropertiesParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertiesParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertiesParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE";
            break;

        case 1:
            o_string = L"CL_QUEUE_PROFILING_ENABLE";
            break;

        case 2:
            o_string = L"CL_QUEUE_ON_DEVICE";
            break;

        case 3:
            o_string = L"CL_QUEUE_ON_DEVICE_DEFAULT";
            break;

        case 63:
            o_string = L"CL_QUEUE_COMMAND_INTERCEPT_ENABLE_AMD";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}




// -----------------------------   apCLMapFlagsParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMapFlagsParameter::type() const
{
    return OS_TOBJ_ID_CL_MAP_FLAGS_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLMapFlagsParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLMapFlagsParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_map_flags)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLMapFlagsParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value =  va_arg(pArgumentList, cl_map_flags);
}


// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void apCLMapFlagsParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_map_flags*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
gtSizeType apCLMapFlagsParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_map_flags);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLMapFlagsParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLMapFlagsParameter* pParam  = (apCLMapFlagsParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLMapFlagsParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLMapFlagsParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapFlagsParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLMapFlagsParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_MAP_READ";
            break;

        case 1:
            o_string = L"CL_MAP_WRITE";
            break;

        case 2:
            o_string = L"CL_MAP_WRITE_INVALIDATE_REGION";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// -----------------------------   apCLMemoryMigrationFlagsParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::type
// Description: Returns my Transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMemoryMigrationFlagsParameter::type() const
{
    return OS_TOBJ_ID_CL_MEM_MIGRATION_FLAGS_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
bool apCLMemoryMigrationFlagsParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
bool apCLMemoryMigrationFlagsParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_mem_migration_flags)valueAsUInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLMemoryMigrationFlagsParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value =  va_arg(pArgumentList, cl_mem_migration_flags);
}


// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
void apCLMemoryMigrationFlagsParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_mem_migration_flags*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
gtSizeType apCLMemoryMigrationFlagsParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_mem_migration_flags);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
bool apCLMemoryMigrationFlagsParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLMemoryMigrationFlagsParameter* pParam  = (apCLMemoryMigrationFlagsParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLMemoryMigrationFlagsParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLMemoryMigrationFlagsParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryMigrationFlagsParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
bool apCLMemoryMigrationFlagsParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_MIGRATE_MEM_OBJECT_HOST";
            break;

        case 1:
            o_string = L"CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ----------------------------- apCLDeviceAffinityDomainParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::type
// Description: Returns my Transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDeviceAffinityDomainParameter::type() const
{
    return OS_TOBJ_ID_CL_DEVICE_AFFINITY_DOMAIN_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceAffinityDomainParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceAffinityDomainParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (cl_device_affinity_domain)valueAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLDeviceAffinityDomainParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_device_affinity_domain);
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void apCLDeviceAffinityDomainParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_device_affinity_domain*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtSizeType apCLDeviceAffinityDomainParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_device_affinity_domain);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceAffinityDomainParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLDeviceAffinityDomainParameter* pParam = (apCLDeviceAffinityDomainParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceAffinityDomainParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLDeviceAffinityDomainParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceAffinityDomainParameter::getBitName
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceAffinityDomainParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_NUMA";
            break;

        case 1:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE";
            break;

        case 2:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE";
            break;

        case 3:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE";
            break;

        case 4:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE";
            break;

        case 5:
            o_string = L"CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ----------------------------- apCLDeviceSVMCapabilitiesParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::type
// Description: Returns my Transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDeviceSVMCapabilitiesParameter::type() const
{
    return OS_TOBJ_ID_CL_DEVICE_SVM_CAPABILITIES_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceSVMCapabilitiesParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceSVMCapabilitiesParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (cl_device_svm_capabilities)valueAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLDeviceSVMCapabilitiesParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_device_svm_capabilities);
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void apCLDeviceSVMCapabilitiesParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_device_svm_capabilities*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtSizeType apCLDeviceSVMCapabilitiesParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_device_svm_capabilities);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceSVMCapabilitiesParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLDeviceSVMCapabilitiesParameter* pParam = (apCLDeviceSVMCapabilitiesParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceSVMCapabilitiesParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLDeviceSVMCapabilitiesParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesParameter::getBitName
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLDeviceSVMCapabilitiesParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_DEVICE_SVM_COARSE_GRAIN_BUFFER";
            break;

        case 1:
            o_string = L"CL_DEVICE_SVM_FINE_GRAIN_BUFFER";
            break;

        case 2:
            o_string = L"CL_DEVICE_SVM_FINE_GRAIN_SYSTEM";
            break;

        case 3:
            o_string = L"CL_DEVICE_SVM_ATOMICS";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

// ----------------------------- apCLKernelArgTypeQualifierParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::type
// Description: Returns my Transferable object type.
// Return Val:  osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLKernelArgTypeQualifierParameter::type() const
{
    return OS_TOBJ_ID_CL_KERNEL_ARG_TYPE_QUALIFIER_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLKernelArgTypeQualifierParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLKernelArgTypeQualifierParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value = (cl_kernel_arg_type_qualifier)valueAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLKernelArgTypeQualifierParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList, cl_kernel_arg_type_qualifier);
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void apCLKernelArgTypeQualifierParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_kernel_arg_type_qualifier*)(pValue));
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtSizeType apCLKernelArgTypeQualifierParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_kernel_arg_type_qualifier);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLKernelArgTypeQualifierParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLKernelArgTypeQualifierParameter* pParam = (apCLKernelArgTypeQualifierParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::getSpecialValue
// Description: Gets the special value strings for CL_NONE
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLKernelArgTypeQualifierParameter::getSpecialValue(gtString& o_string) const
{
    bool retVal = false;

    if (CL_NONE == _value)
    {
        retVal = true;
        o_string = L"CL_KERNEL_ARG_TYPE_NONE";
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::getBitfieldAsUInt64
// Description: Returns the variable value.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
gtUInt64 apCLKernelArgTypeQualifierParameter::getBitfieldAsUInt64() const
{
    return (gtUInt64)_value;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernelArgTypeQualifierParameter::getBitName
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLKernelArgTypeQualifierParameter::getBitName(int bitIndex, gtString& o_string) const
{
    bool retVal = true;

    switch (bitIndex)
    {
        case 0:
            o_string = L"CL_KERNEL_ARG_TYPE_CONST";
            break;

        case 1:
            o_string = L"CL_KERNEL_ARG_TYPE_RESTRICT";
            break;

        case 2:
            o_string = L"CL_KERNEL_ARG_TYPE_VOLATILE";
            break;

        case 3:
            o_string = L"CL_KERNEL_ARG_TYPE_PIPE";
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}


// -----------------------------   apCLEnumParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLEnumParameter::type() const
{
    return OS_TOBJ_ID_CL_ENUM_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLEnumParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLEnumParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value = (cl_uint)valueAsUInt32;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLEnumParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    _value = va_arg(pArgumentList , cl_uint);
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
void apCLEnumParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_uint*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
gtSizeType apCLEnumParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_uint);
    return sizeOfValue;
}


// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// TO_DO: OpenCL The commented out parameters are enumerations that use the same value,
//        therefore cannot be used within the same switch loop. Currently none of them is used
//        in the OpenCL functions, so I ignore them
// ---------------------------------------------------------------------------
void apCLEnumParameter::valueAsString(gtString& valueString) const
{
    bool rcVal = apCLEnumValueToString(_value, valueString);

    if (!rcVal)
    {
        // Unknown enum:
        gtString unknownEnumString;
        unknownEnumString.appendFormattedString(L"Unknown enum: 0x%X", _value);
        GT_ASSERT_EX(false, unknownEnumString.asCharArray());

        valueString = L"Unknown";
    }


}

// ---------------------------------------------------------------------------
// Name:        apCLEnumParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
bool apCLEnumParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLEnumParameter* pParam  = (apCLEnumParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = (_value == pParam->_value);
        }
    }

    return retVal;
}

// -----------------------------   apCLBufferRegionParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLBufferRegionParameter::type() const
{
    return OS_TOBJ_ID_CL_BUFFER_REGION_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool apCLBufferRegionParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_value.origin;
    ipcChannel << (gtUInt64)_value.size;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool apCLBufferRegionParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value.origin = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.size = (size_t)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLBufferRegionParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    void* pPointer = va_arg(pArgumentList, void*);

    if (pPointer != NULL)
    {
        cl_buffer_region* pBufferRegion = (cl_buffer_region*)pPointer;
        _value.origin = pBufferRegion->origin;
        _value.size = pBufferRegion->size;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void apCLBufferRegionParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_buffer_region*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
gtSizeType apCLBufferRegionParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_buffer_region);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void apCLBufferRegionParameter::valueAsHexString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"0x%08x, 0x%08x", (gtUInt32)_value.origin, (gtUInt32)_value.size);
}

// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void apCLBufferRegionParameter::valueAsString(gtString& valueString) const
{
    valueString.makeEmpty();
    valueString.appendFormattedString(L"%u, %u", _value.origin, _value.size);
}

// ---------------------------------------------------------------------------
// Name:        apCLBufferRegionParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool apCLBufferRegionParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLBufferRegionParameter* pParam  = (apCLBufferRegionParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = ((_value.origin == pParam->_value.origin) && (_value.size == _value.size));
        }
    }

    return retVal;
}

// -----------------------------   apCLImageDescriptionParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::apCLImageDescriptionParameter
// Description: Constructor
// Arguments:   const cl_image_desc& imageDesc
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
apCLImageDescriptionParameter::apCLImageDescriptionParameter(const cl_image_desc& imageDesc)
{
    _value.image_type = imageDesc.image_type;
    _value.image_width = imageDesc.image_width;
    _value.image_height = imageDesc.image_height;
    _value.image_depth = imageDesc.image_depth;
    _value.image_array_size = imageDesc.image_array_size;
    _value.image_row_pitch = imageDesc.image_row_pitch;
    _value.image_slice_pitch = imageDesc.image_slice_pitch;
    _value.num_mip_levels = imageDesc.num_mip_levels;
    _value.num_samples = imageDesc.num_samples;
    _value.buffer = imageDesc.buffer;
}


// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
osTransferableObjectType apCLImageDescriptionParameter::type() const
{
    return OS_TOBJ_ID_CL_IMAGE_DESCRIPTION_PARAMETER;
}


// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::writeSelfIntoChannel
// Description: Writes the parameter value into an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
bool apCLImageDescriptionParameter::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt32)_value.image_type;
    ipcChannel << (gtUInt64)_value.image_width;
    ipcChannel << (gtUInt64)_value.image_height;
    ipcChannel << (gtUInt64)_value.image_depth;
    ipcChannel << (gtUInt64)_value.image_array_size;
    ipcChannel << (gtUInt64)_value.image_row_pitch;
    ipcChannel << (gtUInt64)_value.image_slice_pitch;
    ipcChannel << (gtUInt32)_value.num_mip_levels;
    ipcChannel << (gtUInt32)_value.num_samples;
    ipcChannel << (gtUInt64)_value.buffer;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::readSelfFromChannel
// Description: Reads the parameter value from an IPC channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
bool apCLImageDescriptionParameter::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt32 valueAsUInt32 = 0;
    ipcChannel >> valueAsUInt32;
    _value.image_type = (cl_mem_object_type)valueAsUInt32;

    gtUInt64 valueAsUInt64 = 0;
    ipcChannel >> valueAsUInt64;
    _value.image_width = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.image_height = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.image_depth = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.image_array_size = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.image_row_pitch = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt64;
    _value.image_slice_pitch = (size_t)valueAsUInt64;

    ipcChannel >> valueAsUInt32;
    _value.num_mip_levels = (size_t)valueAsUInt32;

    ipcChannel >> valueAsUInt32;
    _value.num_samples = (size_t)valueAsUInt32;

    ipcChannel >> valueAsUInt64;
    _value.buffer = (cl_mem)valueAsUInt64;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::readValueFromArgumentsList
// Description: Reads my value from an stdarg argument list.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// Implementation notes: See "The use of stdarg" at the top of this file.
// ---------------------------------------------------------------------------
void apCLImageDescriptionParameter::readValueFromArgumentsList(va_list& pArgumentList)
{
    void* pPointer = va_arg(pArgumentList, void*);

    if (pPointer != NULL)
    {
        cl_image_desc* pImageDesc = (cl_image_desc*)pPointer;

        _value.image_type = pImageDesc->image_type;
        _value.image_width = pImageDesc->image_width;
        _value.image_height = pImageDesc->image_height;
        _value.image_depth = pImageDesc->image_depth;
        _value.image_array_size = pImageDesc->image_array_size;
        _value.image_row_pitch = pImageDesc->image_row_pitch;
        _value.image_slice_pitch = pImageDesc->image_slice_pitch;
        _value.num_mip_levels = pImageDesc->num_mip_levels;
        _value.num_samples = pImageDesc->num_samples;
        _value.buffer = pImageDesc->buffer;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::readValueFromPointer
// Description: Reads my value from a pointer to a value.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
void apCLImageDescriptionParameter::readValueFromPointer(void* pValue)
{
    _value = *((cl_image_desc*)(pValue));
}


// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::sizeofData
// Description: Returns my data size.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
gtSizeType apCLImageDescriptionParameter::sizeofData()
{
    static gtSizeType sizeOfValue = sizeof(cl_image_desc);
    return sizeOfValue;
}

// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::valueAsHexString
// Description: Returns the parameter value as a string in hexadecimal display
// Arguments:   gtString& valueString
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
void apCLImageDescriptionParameter::valueAsHexString(gtString& valueString) const
{
    apCLEnumParameter enumParam(_value.image_type);
    enumParam.valueAsString(valueString);
    valueString.appendFormattedString(L", w:0x%08x, h:0x%08x, d: 0x%08x, a: 0x%08x, rp: 0x%08x, sp: 0x%08x, ml: 0x%08x, sm: 0x%08x, buf:0x%16llx }", (gtUInt32)_value.image_width,
                                      (gtUInt32)_value.image_height, (gtUInt32)_value.image_depth,
                                      (gtUInt32)_value.image_array_size, (gtUInt32)_value.image_row_pitch,
                                      (gtUInt32)_value.image_slice_pitch, (gtUInt32)_value.num_mip_levels,
                                      (gtUInt32)_value.num_samples, (gtUInt64)_value.buffer).prepend('{');
}

// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::valueAsString
// Description: Returns the parameter value as a string.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
void apCLImageDescriptionParameter::valueAsString(gtString& valueString) const
{
    apCLEnumParameter enumParam(_value.image_type);
    enumParam.valueAsString(valueString);
    valueString.appendFormattedString(L", w:%u, h:%u, d: %u, a: %u, rp: %u, sp: %u, ml: %u, sm: %u, buf:0x%16llx }", (gtUInt32)_value.image_width,
                                      (gtUInt32)_value.image_height, (gtUInt32)_value.image_depth,
                                      (gtUInt32)_value.image_array_size, (gtUInt32)_value.image_row_pitch,
                                      (gtUInt32)_value.image_slice_pitch, (gtUInt32)_value.num_mip_levels,
                                      (gtUInt32)_value.num_samples, (gtUInt64)_value.buffer).prepend('{');
}

// ---------------------------------------------------------------------------
// Name:        apCLImageDescriptionParameter::compareToOther
// Description: Compares this with other
// Arguments: const& apParameter other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/1/2012
// ---------------------------------------------------------------------------
bool apCLImageDescriptionParameter::compareToOther(const apParameter& other)const
{
    bool retVal = false;

    // Compare types:
    if (this->type() == other.type())
    {
        // Down cast to apPointerParameter:
        apCLImageDescriptionParameter* pParam  = (apCLImageDescriptionParameter*)(&other);
        GT_IF_WITH_ASSERT(pParam != NULL)
        {
            retVal = ((_value.image_type == pParam->_value.image_type) && (_value.image_width == _value.image_width) &&
                      (_value.image_height == pParam->_value.image_height) && (_value.image_depth == _value.image_depth) &&
                      (_value.image_array_size == pParam->_value.image_array_size) && (_value.image_row_pitch == _value.image_row_pitch) &&
                      (_value.image_slice_pitch == pParam->_value.image_slice_pitch) && (_value.num_mip_levels == _value.num_mip_levels) &&
                      (_value.num_samples == pParam->_value.num_samples) && (_value.buffer == _value.buffer));
        }
    }

    return retVal;
}

// ----------------------------- apCLContextPropertyListParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLContextPropertyListParameter::apCLContextPropertyListParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apCLContextPropertyListParameter::apCLContextPropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLContextPropertyListParameter::~apCLContextPropertyListParameter
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
apCLContextPropertyListParameter::~apCLContextPropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLContextPropertyListParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLContextPropertyListParameter::type() const
{
    return OS_TOBJ_ID_CL_CONTEXT_PROPERTIES_LIST_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLContextPropertyListParameter::sizeOfArrayElement
// Description: Returns the size of a single array element
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
unsigned int apCLContextPropertyListParameter::sizeOfArrayElement() const
{
    return sizeof(cl_context_properties);
}

// ---------------------------------------------------------------------------
// Name:        apCLContextPropertyListParameter::fillParamNameAndValue
// Description: Fills the parameters with values from cl_context_properties
// Author:  AMD Developer Tools Team
// Date:        28/9/2014
// ---------------------------------------------------------------------------
bool apCLContextPropertyListParameter::fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const
{
    bool retVal = (NULL != i_pArrayItem1) && (NULL != i_pArrayItem2);

    GT_IF_WITH_ASSERT(retVal)
    {
        const cl_context_properties& property = *(cl_context_properties*)i_pArrayItem1;
        o_aptrArrayItem1 = new apCLEnumParameter((cl_uint)property);

        switch (property)
        {
            case CL_CONTEXT_PLATFORM:
                // o_aptrArrayItem2 = new apCLPlatformIDParameter(*(cl_platform_id*)i_pArrayItem2);
                o_aptrArrayItem2 = new apCLHandleParameter(*(oaCLPlatformID*)i_pArrayItem2);
                break;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            // D3D is only supported on Windows:
            case CL_CONTEXT_ADAPTER_D3D9_KHR:
            case CL_CONTEXT_ADAPTER_D3D9EX_KHR:
            case CL_CONTEXT_ADAPTER_DXVA_KHR:
            case CL_CONTEXT_D3D10_DEVICE_KHR:
            case CL_CONTEXT_D3D11_DEVICE_KHR:
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;

            case CL_GL_CONTEXT_KHR:
                // o_aptrArrayItem2 = new apWin32HGLRCParameter(*(HGLRC*)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER);
                break;

            case CL_WGL_HDC_KHR:
                // o_aptrArrayItem2 = new apWin32HDCParameter(*(HDC*)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2, OS_TOBJ_ID_WIN32_HDC_PARAMETER);
                break;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT

            case CL_GL_CONTEXT_KHR:
                // o_aptrArrayItem2 = new apGLXContextParamter(*(GLXContext*)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;

            case CL_GLX_DISPLAY_KHR:
                // o_aptrArrayItem2 = new apXDisplayParameter(*(Display**)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

            case CL_GL_CONTEXT_KHR:
#ifdef _GR_IPHONE_BUILD
                // o_aptrArrayItem2 = new apEAGLContextParameter(*(EAGLContext*)i_pArrayItem2);
#else
                // o_aptrArrayItem2 = new apCGLContextObjParameter(*(CGLContextObj*)i_pArrayItem2);
#endif
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;

            case CL_EGL_DISPLAY_KHR:
                // o_aptrArrayItem2 = new apEGLDisplayParameter(*(EGLDisplay*)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;

            case CL_CGL_SHAREGROUP_KHR:
                // o_aptrArrayItem2 = new apCGLContextObjParameter(*(CGLContextObj*)i_pArrayItem2);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
#else
#error Unknown Linux Variant!
#endif
#else
#error Unknown build target!
#endif

            default:
                // Unknown parameter, treat the value as a pointer:
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
        }
    }

    return retVal;
}

// ----------------------------- apCLCommandQueuePropertyListParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertyListParameter::apCLCommandQueuePropertyListParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLCommandQueuePropertyListParameter::apCLCommandQueuePropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertyListParameter::~apCLCommandQueuePropertyListParameter
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLCommandQueuePropertyListParameter::~apCLCommandQueuePropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertyListParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCommandQueuePropertyListParameter::type() const
{
    return OS_TOBJ_ID_CL_COMMAND_QUEUE_PROPERTIES_LIST_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertyListParameter::sizeOfArrayElement
// Description: Returns the size of a single array element
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
unsigned int apCLCommandQueuePropertyListParameter::sizeOfArrayElement() const
{
    return sizeof(cl_command_queue_properties);
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueuePropertyListParameter::fillParamNameAndValue
// Description: Fills the parameters with values from cl_CommandQueue_properties
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLCommandQueuePropertyListParameter::fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const
{
    bool retVal = (NULL != i_pArrayItem1) && (NULL != i_pArrayItem2);

    GT_IF_WITH_ASSERT(retVal)
    {
        const cl_command_queue_properties& property = *(cl_command_queue_properties*)i_pArrayItem1;
        o_aptrArrayItem1 = new apCLEnumParameter((cl_uint)property);

        switch (property)
        {
            case CL_QUEUE_PROPERTIES:
                o_aptrArrayItem2 = new apCLCommandQueuePropertiesParameter(*(cl_command_queue_properties*)i_pArrayItem2);
                break;

            case CL_QUEUE_SIZE:
                o_aptrArrayItem2 = new apCLuintParameter(*(cl_uint*)i_pArrayItem2);
                break;

            default:
                // Unknown parameter, treat the value as a pointer:
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
        }
    }

    return retVal;
}

// ----------------------------- apCLPipePropertyListParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLPipePropertyListParameter::apCLPipePropertyListParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLPipePropertyListParameter::apCLPipePropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLPipePropertyListParameter::~apCLPipePropertyListParameter
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLPipePropertyListParameter::~apCLPipePropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLPipePropertyListParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLPipePropertyListParameter::type() const
{
    return OS_TOBJ_ID_CL_PIPE_PROPERTIES_LIST_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLPipePropertyListParameter::sizeOfArrayElement
// Description: Returns the size of a single array element
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
unsigned int apCLPipePropertyListParameter::sizeOfArrayElement() const
{
    return sizeof(cl_pipe_properties);
}

// ---------------------------------------------------------------------------
// Name:        apCLPipePropertyListParameter::fillParamNameAndValue
// Description: Fills the parameters with values from cl_pipe_properties
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLPipePropertyListParameter::fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const
{
    bool retVal = (NULL != i_pArrayItem1) && (NULL != i_pArrayItem2);

    GT_IF_WITH_ASSERT(retVal)
    {
        const cl_pipe_properties& property = *(cl_pipe_properties*)i_pArrayItem1;
        o_aptrArrayItem1 = new apCLEnumParameter((cl_uint)property);

        switch (property)
        {
            // Uri, 29/9/2014 - OpenCL 2.0 does not define any pipe properties as input.
            //                  As a result, we put this junk case here (which should never be reached), just to have this file compile properly:
            case 0:
                GT_ASSERT(false);
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;

            default:
                // Unknown parameter, treat the value as a pointer:
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
        }
    }

    return retVal;
}

// ----------------------------- apCLSamplerPropertyListParameter ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSamplerPropertyListParameter::apCLSamplerPropertyListParameter
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLSamplerPropertyListParameter::apCLSamplerPropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSamplerPropertyListParameter::~apCLSamplerPropertyListParameter
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
apCLSamplerPropertyListParameter::~apCLSamplerPropertyListParameter()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSamplerPropertyListParameter::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSamplerPropertyListParameter::type() const
{
    return OS_TOBJ_ID_CL_SAMPLER_PROPERTIES_LIST_PARAMETER;
}

// ---------------------------------------------------------------------------
// Name:        apCLSamplerPropertyListParameter::sizeOfArrayElement
// Description: Returns the size of a single array element
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
unsigned int apCLSamplerPropertyListParameter::sizeOfArrayElement() const
{
    return sizeof(cl_sampler_properties);
}

// ---------------------------------------------------------------------------
// Name:        apCLSamplerPropertyListParameter::fillParamNameAndValue
// Description: Fills the parameters with values from cl_sampler_properties
// Author:  AMD Developer Tools Team
// Date:        29/9/2014
// ---------------------------------------------------------------------------
bool apCLSamplerPropertyListParameter::fillParamNameAndValue(const void* i_pArrayItem1, const void* i_pArrayItem2, gtAutoPtr<apParameter>& o_aptrArrayItem1, gtAutoPtr<apParameter>& o_aptrArrayItem2) const
{
    bool retVal = (NULL != i_pArrayItem1) && (NULL != i_pArrayItem2);

    GT_IF_WITH_ASSERT(retVal)
    {
        const cl_sampler_properties& property = *(cl_sampler_properties*)i_pArrayItem1;
        o_aptrArrayItem1 = new apCLEnumParameter((cl_uint)property);

        switch (property)
        {
            case CL_SAMPLER_NORMALIZED_COORDS:
                o_aptrArrayItem2 = new apCLBoolParameter(*(cl_bool*)i_pArrayItem2);
                break;

            case CL_SAMPLER_ADDRESSING_MODE:
                o_aptrArrayItem2 = new apCLEnumParameter(*(cl_addressing_mode*)i_pArrayItem2);
                break;

            case CL_SAMPLER_FILTER_MODE:
                o_aptrArrayItem2 = new apCLEnumParameter(*(cl_filter_mode*)i_pArrayItem2);
                break;

            case CL_SAMPLER_MIP_FILTER_MODE:
                o_aptrArrayItem2 = new apCLEnumParameter(*(cl_filter_mode*)i_pArrayItem2);
                break;

            case CL_SAMPLER_LOD_MIN:
            case CL_SAMPLER_LOD_MAX:
                o_aptrArrayItem2 = new apFloatParameter(*(float*)i_pArrayItem2);
                break;

            default:
                // Unknown parameter, treat the value as a pointer:
                o_aptrArrayItem2 = new apPointerParameter(*(void**)i_pArrayItem2);
                break;
        }
    }

    return retVal;
}





