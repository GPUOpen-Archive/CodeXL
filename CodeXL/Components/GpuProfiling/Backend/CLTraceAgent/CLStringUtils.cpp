//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions to stringify CL params.
//==============================================================================

#ifdef _WIN32
    #include <CL/cl_d3d11.h>
    #include <CL/cl_d3d10.h>
    #include <CL/cl_dx9_media_sharing.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h> // needed for "GL_TEXTURE_" constants used in GetGLTextureTargetString

#include "CLStringUtils.h"
#include "CLTraceAgent.h"
#include "../Common/Defs.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"

std::string CLStringUtils::GetBoolString(const cl_bool b)
{
    return (b == CL_TRUE) ? "CL_TRUE" : "CL_FALSE";
}

std::string CLStringUtils::GetSizeListString(const size_t* sizes, size_t count, bool include_brackets)
{
    if (sizes == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    for (size_t i = 0; i < count; i++)
    {
        ss << sizes[i];

        if (i < count - 1)
        {
            ss << ',';
        }
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetNDimString(const size_t* nd, size_t dims)
{
    if (dims > 3)
    {
        dims = 3;
    }

    return GetSizeListString(nd, dims);
}

std::string CLStringUtils::GetErrorString(const cl_int errcode)
{
    switch (errcode)
    {
            CASE(CL_SUCCESS);
            CASE(CL_DEVICE_NOT_FOUND);
            CASE(CL_DEVICE_NOT_AVAILABLE);
            CASE(CL_COMPILER_NOT_AVAILABLE);
            CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            CASE(CL_OUT_OF_RESOURCES);
            CASE(CL_OUT_OF_HOST_MEMORY);
            CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
            CASE(CL_MEM_COPY_OVERLAP);
            CASE(CL_IMAGE_FORMAT_MISMATCH);
            CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
            CASE(CL_BUILD_PROGRAM_FAILURE);
            CASE(CL_MAP_FAILURE);
            CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET);
            CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            CASE(CL_COMPILE_PROGRAM_FAILURE);
            CASE(CL_LINKER_NOT_AVAILABLE);
            CASE(CL_LINK_PROGRAM_FAILURE);
            CASE(CL_DEVICE_PARTITION_FAILED);
            CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
            CASE(CL_INVALID_VALUE);
            CASE(CL_INVALID_DEVICE_TYPE);
            CASE(CL_INVALID_PLATFORM);
            CASE(CL_INVALID_DEVICE);
            CASE(CL_INVALID_CONTEXT);
            CASE(CL_INVALID_QUEUE_PROPERTIES);
            CASE(CL_INVALID_COMMAND_QUEUE);
            CASE(CL_INVALID_HOST_PTR);
            CASE(CL_INVALID_MEM_OBJECT);
            CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
            CASE(CL_INVALID_IMAGE_SIZE);
            CASE(CL_INVALID_SAMPLER);
            CASE(CL_INVALID_BINARY);
            CASE(CL_INVALID_BUILD_OPTIONS);
            CASE(CL_INVALID_PROGRAM);
            CASE(CL_INVALID_PROGRAM_EXECUTABLE);
            CASE(CL_INVALID_KERNEL_NAME);
            CASE(CL_INVALID_KERNEL_DEFINITION);
            CASE(CL_INVALID_KERNEL);
            CASE(CL_INVALID_ARG_INDEX);
            CASE(CL_INVALID_ARG_VALUE);
            CASE(CL_INVALID_ARG_SIZE);
            CASE(CL_INVALID_KERNEL_ARGS);
            CASE(CL_INVALID_WORK_DIMENSION);
            CASE(CL_INVALID_WORK_GROUP_SIZE);
            CASE(CL_INVALID_WORK_ITEM_SIZE);
            CASE(CL_INVALID_GLOBAL_OFFSET);
            CASE(CL_INVALID_EVENT_WAIT_LIST);
            CASE(CL_INVALID_EVENT);
            CASE(CL_INVALID_OPERATION);
            CASE(CL_INVALID_GL_OBJECT);
            CASE(CL_INVALID_BUFFER_SIZE);
            CASE(CL_INVALID_MIP_LEVEL);
            CASE(CL_INVALID_GLOBAL_WORK_SIZE);
            CASE(CL_INVALID_PROPERTY);
            CASE(CL_INVALID_IMAGE_DESCRIPTOR);
            CASE(CL_INVALID_COMPILER_OPTIONS);
            CASE(CL_INVALID_LINKER_OPTIONS);
            CASE(CL_INVALID_DEVICE_PARTITION_COUNT);
            CASE(CL_INVALID_PIPE_SIZE);
            CASE(CL_INVALID_DEVICE_QUEUE);
            CASE(CL_DEVICE_PARTITION_FAILED_EXT);
            CASE(CL_INVALID_PARTITION_COUNT_EXT);
            CASE(CL_INVALID_PARTITION_NAME_EXT);
            CASE(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR);
            CASE(CL_PLATFORM_NOT_FOUND_KHR);
            // CASE(CL_INVALID_COUNTER_AMD);  // AMD extension for atomic counters was removed in CL 646660 of OpenCL staging tree
#ifdef _WIN32
            CASE(CL_INVALID_D3D10_DEVICE_KHR);
            CASE(CL_INVALID_D3D10_RESOURCE_KHR);
            CASE(CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR);
            CASE(CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR);
#endif

        default: return StringUtils::ToString(errcode);
    }
}

std::string CLStringUtils::GetMemObjectTypeString(const cl_mem_object_type type)
{
    switch (type)
    {
            CASE(CL_MEM_OBJECT_BUFFER);
            CASE(CL_MEM_OBJECT_IMAGE2D);
            CASE(CL_MEM_OBJECT_IMAGE3D);
            CASE(CL_MEM_OBJECT_IMAGE2D_ARRAY);
            CASE(CL_MEM_OBJECT_IMAGE1D);
            CASE(CL_MEM_OBJECT_IMAGE1D_ARRAY);
            CASE(CL_MEM_OBJECT_IMAGE1D_BUFFER);
            CASE(CL_MEM_OBJECT_PIPE);

        default: return StringUtils::ToString(type);
    }
}

std::string CLStringUtils::GetMemInfoString(const cl_mem_info param_name)
{
    switch (param_name)
    {
            CASE(CL_MEM_TYPE);
            CASE(CL_MEM_FLAGS);
            CASE(CL_MEM_SIZE);
            CASE(CL_MEM_HOST_PTR);
            CASE(CL_MEM_MAP_COUNT);
            CASE(CL_MEM_REFERENCE_COUNT);
            CASE(CL_MEM_CONTEXT);
            CASE(CL_MEM_ASSOCIATED_MEMOBJECT);
            CASE(CL_MEM_OFFSET);
            CASE(CL_MEM_USES_SVM_POINTER);
#ifdef _WIN32
            CASE(CL_MEM_D3D10_RESOURCE_KHR);
#endif

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetMemInfoValueString(const cl_mem_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_MEM_TYPE:
                    ss << GetMemObjectTypeString(*((cl_mem_object_type*)param_value));
                    break;

                case CL_MEM_FLAGS:
                    ss << GetMemFlagsString(*((cl_mem_flags*)param_value));
                    break;

                case CL_MEM_SIZE:
                    ss << *((size_t*)param_value);
                    break;

                case CL_MEM_HOST_PTR:
                    ss << StringUtils::ToHexString((void*)(*(intptr_t*)param_value));
                    break;

                case CL_MEM_MAP_COUNT:
                case CL_MEM_REFERENCE_COUNT:
                    ss << *((cl_uint*)param_value);
                    break;

                case CL_MEM_CONTEXT:
                    ss << StringUtils::ToHexString(*((cl_context*)param_value));
                    break;

                case CL_MEM_ASSOCIATED_MEMOBJECT:
                    ss << StringUtils::ToHexString(*((cl_mem*)param_value));
                    break;

                case CL_MEM_OFFSET:
                    ss << *((size_t*)param_value);
                    break;

                case CL_MEM_USES_SVM_POINTER:
                    ss << GetBoolString(*(cl_bool*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetImageInfoString(const cl_image_info param_name)
{
    switch (param_name)
    {
            CASE(CL_IMAGE_FORMAT);
            CASE(CL_IMAGE_ELEMENT_SIZE);
            CASE(CL_IMAGE_ROW_PITCH);
            CASE(CL_IMAGE_SLICE_PITCH);
            CASE(CL_IMAGE_WIDTH);
            CASE(CL_IMAGE_HEIGHT);
            CASE(CL_IMAGE_DEPTH);
            CASE(CL_IMAGE_ARRAY_SIZE);
            CASE(CL_IMAGE_BUFFER);
            CASE(CL_IMAGE_NUM_MIP_LEVELS);
            CASE(CL_IMAGE_NUM_SAMPLES);
            CASE(CL_IMAGE_BYTE_PITCH_AMD);
#ifdef _WIN32
            CASE(CL_IMAGE_D3D10_SUBRESOURCE_KHR);
#endif

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetImageInfoValueString(const cl_image_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_IMAGE_FORMAT:
                    ss << GetImageFormatsString((cl_image_format*)param_value, 1, false);
                    break;

                case CL_IMAGE_ELEMENT_SIZE:
                case CL_IMAGE_ROW_PITCH:
                case CL_IMAGE_SLICE_PITCH:
                case CL_IMAGE_WIDTH:
                case CL_IMAGE_HEIGHT:
                case CL_IMAGE_DEPTH:
                case CL_IMAGE_ARRAY_SIZE:
                case CL_IMAGE_NUM_MIP_LEVELS:
                case CL_IMAGE_NUM_SAMPLES:
                case CL_IMAGE_BYTE_PITCH_AMD:
                    ss << *((size_t*)param_value);
                    break;

                case CL_IMAGE_BUFFER:
                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetPipeInfoString(const cl_pipe_info param_name)
{
    switch (param_name)
    {
            CASE(CL_PIPE_PACKET_SIZE);
            CASE(CL_PIPE_MAX_PACKETS);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetPipeInfoValueString(const cl_pipe_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_PIPE_PACKET_SIZE:
                case CL_PIPE_MAX_PACKETS:
                    ss << *((cl_uint*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetErrorString(const cl_int* errcode_ret, const cl_int errcode_retVal)
{
    if (errcode_ret == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;
    ss << '[' << GetErrorString(errcode_retVal) << ']';
    return ss.str();
}

std::string CLStringUtils::GetErrorStrings(const cl_int* errcode_ret, cl_uint num_errcodes)
{
    if (errcode_ret == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;
    ss << '[';

    for (cl_uint i = 0; i < num_errcodes; i++)
    {
        if (i != 0)
        {
            ss << ',';
        }

        ss << GetErrorString(errcode_ret[i]);
    }

    ss << ']';
    return ss.str();
}

std::string CLStringUtils::GetHandlesString(const void* handles, cl_uint num_handles)
{
    if (handles == NULL)
    {
        return "NULL";
    }

    if (num_handles == 0)
    {
        return "[]";
    }

    const cl_event* p = reinterpret_cast<const cl_event*>(handles);

    std::ostringstream ss;

    ss << '[';

    while (num_handles > 0)
    {
        ss << StringUtils::ToHexString(*p);
        p++;

        if (--num_handles != 0)
        {
            ss << ',';
        }
    }

    ss << ']';
    return ss.str();
}

std::string CLStringUtils::GetContextPropertyString(const cl_context_properties cprop)
{
    switch (cprop)
    {
            CASE(CL_CONTEXT_PLATFORM);
            CASE(CL_CONTEXT_INTEROP_USER_SYNC);
            CASE(CL_GL_CONTEXT_KHR);
            CASE(CL_EGL_DISPLAY_KHR);
            CASE(CL_GLX_DISPLAY_KHR);
            CASE(CL_WGL_HDC_KHR);
            CASE(CL_CGL_SHAREGROUP_KHR);
            CASE(CL_CONTEXT_OFFLINE_DEVICES_AMD);
            CASE(CL_CONTEXT_MEMORY_INITIALIZE_KHR);
            CASE(CL_DEVICE_TERMINATE_CAPABILITY_KHR);
            CASE(CL_CONTEXT_TERMINATE_KHR);
#ifdef _WIN32
            CASE(CL_CONTEXT_D3D10_DEVICE_KHR);
            CASE(CL_CONTEXT_D3D10_PREFER_SHARED_RESOURCES_KHR);
            CASE(CL_CONTEXT_ADAPTER_D3D9_KHR);
            CASE(CL_CONTEXT_ADAPTER_D3D9EX_KHR);
            CASE(CL_CONTEXT_ADAPTER_DXVA_KHR);
            CASE(CL_CONTEXT_D3D11_DEVICE_KHR);
#endif

        default: return StringUtils::ToString(cprop);
    }
}

std::string CLStringUtils::GetContextPropertiesString(const cl_context_properties* pProperties,
                                                      const std::vector<cl_context_properties>& properties,
                                                      bool include_brackets)
{
    if (pProperties == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << '{';

    int num_properties = 0;

    for (std::vector<cl_context_properties>::const_iterator it = properties.begin(); it != properties.end() && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES; it++)
    {
        ss << GetContextPropertyString(*it) << ',';
        num_properties++;

        if (++it == properties.end())
        {
            break;
        }
        else
        {
            ss << StringUtils::ToString(*it) << ",";
            num_properties++;
        }
    }

    if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
    {
        ss << "...}";
    }
    else
    {
        ss << "NULL}";
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetCommandQueuePropertyString(const cl_command_queue_properties property)
{
    if (property == 0)
    {
        return "0";
    }

    std::ostringstream ss;

    cl_command_queue_properties props = property;

    while (props)
    {
        if (props & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
        {
            ss << "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE";
            props &= ~CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        }
        else if (props & CL_QUEUE_PROFILING_ENABLE)
        {
            ss << "CL_QUEUE_PROFILING_ENABLE";
            props &= ~CL_QUEUE_PROFILING_ENABLE;
        }
        else if (props & CL_QUEUE_ON_DEVICE)
        {
            ss << "CL_QUEUE_ON_DEVICE";
            props &= ~CL_QUEUE_ON_DEVICE;
        }
        else if (props & CL_QUEUE_ON_DEVICE_DEFAULT)
        {
            ss << "CL_QUEUE_ON_DEVICE_DEFAULT";
            props &= ~CL_QUEUE_ON_DEVICE_DEFAULT;
        }
        else
        {
            ss << StringUtils::ToString(props);
            props = 0;
        }

        if (props != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetCommandQueuePropertiesString(const cl_command_queue_properties* pProperties, const cl_command_queue_properties property, bool include_brackets)
{
    if (pProperties == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << GetCommandQueuePropertyString(property);

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetCommandQueuePropertiesString(const cl_queue_properties* pProperties, std::vector<cl_queue_properties> properties, bool include_brackets)
{
    if (pProperties == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << '{';

    int num_properties = 0;

    for (std::vector<cl_queue_properties>::const_iterator it = properties.begin(); it != properties.end() && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES; it++)
    {
        cl_command_queue_info queueInfo = (cl_command_queue_info)(*it);

        ss << GetCommandQueueInfoString(queueInfo) << ',';
        num_properties++;

        if (++it == properties.end())
        {
            break;
        }
        else
        {
            if (queueInfo == CL_QUEUE_PROPERTIES)
            {
                cl_command_queue_properties queueProperties = (cl_command_queue_properties)(*it);
                ss << GetCommandQueuePropertiesString(&queueProperties, queueProperties, false) << ",";
            }
            else if (queueInfo == CL_QUEUE_SIZE)
            {
                ss << (cl_uint)(*it) << ",";
            }
            else
            {
                ss << StringUtils::ToString(*it) << ",";
            }

            num_properties++;
        }
    }

    if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
    {
        ss << "...}";
    }
    else
    {
        ss << "NULL}";
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetPipePropertyString(const cl_pipe_properties prop)
{
    if (prop == 0)
    {
        return "0";
    }

    std::ostringstream ss;

    cl_pipe_properties props = prop;

    while (props)
    {
        // TODO: For OpenCL 2.0 cl_pipe_properties MUST be NULL.  We need to revisit this in future OCL releases if it is allowed to be non-NULL.
        //       For an example of what this implemention would look like for a non-NULL value, see CLStringUtils::GetCommandQueuePropertyString
        ss << StringUtils::ToString(props);
        props = 0;
    }

    return ss.str();
}

std::string CLStringUtils::GetPipePropertiesString(const cl_pipe_properties* pProperties, const std::vector<cl_pipe_properties>& properties, bool include_brackets)
{
    if (pProperties == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << '{';

    int num_properties = 0;

    for (std::vector<cl_pipe_properties>::const_iterator it = properties.begin(); it != properties.end() && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES; it++)
    {
        cl_pipe_info pipeInfo = (cl_pipe_info)(*it);

        ss << GetPipePropertyString(pipeInfo) << ',';
        num_properties++;

        if (++it == properties.end())
        {
            break;
        }
        else
        {
            // TODO: For OpenCL 2.0 cl_pipe_properties MUST be NULL.  We need to revisit this in future OCL releases if it is allowed to be non-NULL.
            //       For an example of what this implemention would look like for a non-NULL value, see CLStringUtils::GetCommandQueuePropertiesString
            ss << StringUtils::ToString(*it) << ",";

            num_properties++;
        }
    }

    if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
    {
        ss << "...}";
    }
    else
    {
        ss << "NULL}";
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetMemFlagsString(const cl_mem_flags flags)
{
    if (flags == 0)
    {
        return "0";
    }

    std::ostringstream ss;
    cl_mem_flags tmpFlag = flags;

    while (tmpFlag)
    {
        if (tmpFlag & CL_MEM_READ_WRITE)
        {
            ss << "CL_MEM_READ_WRITE";
            tmpFlag &= ~CL_MEM_READ_WRITE;
        }
        else if (tmpFlag & CL_MEM_WRITE_ONLY)
        {
            ss << "CL_MEM_WRITE_ONLY";
            tmpFlag &= ~CL_MEM_WRITE_ONLY;
        }
        else if (tmpFlag & CL_MEM_READ_ONLY)
        {
            ss << "CL_MEM_READ_ONLY";
            tmpFlag &= ~CL_MEM_READ_ONLY;
        }
        else if (tmpFlag & CL_MEM_USE_HOST_PTR)
        {
            ss << "CL_MEM_USE_HOST_PTR";
            tmpFlag &= ~CL_MEM_USE_HOST_PTR;
        }
        else if (tmpFlag & CL_MEM_ALLOC_HOST_PTR)
        {
            ss << "CL_MEM_ALLOC_HOST_PTR";
            tmpFlag &= ~CL_MEM_ALLOC_HOST_PTR;
        }
        else if (tmpFlag & CL_MEM_COPY_HOST_PTR)
        {
            ss << "CL_MEM_COPY_HOST_PTR";
            tmpFlag &= ~CL_MEM_COPY_HOST_PTR;
        }
        else if (tmpFlag & CL_MEM_HOST_WRITE_ONLY)
        {
            ss << "CL_MEM_HOST_WRITE_ONLY";
            tmpFlag &= ~CL_MEM_HOST_WRITE_ONLY;
        }
        else if (tmpFlag & CL_MEM_HOST_READ_ONLY)
        {
            ss << "CL_MEM_HOST_READ_ONLY";
            tmpFlag &= ~CL_MEM_HOST_READ_ONLY;
        }
        else if (tmpFlag & CL_MEM_HOST_NO_ACCESS)
        {
            ss << "CL_MEM_HOST_NO_ACCESS";
            tmpFlag &= ~CL_MEM_HOST_NO_ACCESS;
        }
        else if (tmpFlag & CL_MEM_USE_PERSISTENT_MEM_AMD)
        {
            ss << "CL_MEM_USE_PERSISTENT_MEM_AMD";
            tmpFlag &= ~CL_MEM_USE_PERSISTENT_MEM_AMD;
        }
        else if (tmpFlag & CL_MEM_BUS_ADDRESSABLE_AMD)
        {
            ss << "CL_MEM_BUS_ADDRESSABLE_AMD";
            tmpFlag &= ~CL_MEM_BUS_ADDRESSABLE_AMD;
        }
        else if (tmpFlag & CL_MEM_EXTERNAL_PHYSICAL_AMD)
        {
            ss << "CL_MEM_EXTERNAL_PHYSICAL_AMD";
            tmpFlag &= ~CL_MEM_EXTERNAL_PHYSICAL_AMD;
        }
        else if (tmpFlag & CL_MEM_SVM_FINE_GRAIN_BUFFER)
        {
            ss << "CL_MEM_SVM_FINE_GRAIN_BUFFER";
            tmpFlag &= ~CL_MEM_SVM_FINE_GRAIN_BUFFER;
        }
        else if (tmpFlag & CL_MEM_SVM_ATOMICS)
        {
            ss << "CL_MEM_SVM_ATOMICS";
            tmpFlag &= ~CL_MEM_SVM_ATOMICS;
        }
        else
        {
            ss << StringUtils::ToString(tmpFlag);
            tmpFlag = 0;
        }

        if (tmpFlag != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetMemMigrationFlagsString(const cl_mem_migration_flags flags)
{
    if (flags == 0)
    {
        return "0";
    }

    std::ostringstream ss;
    cl_mem_flags tmpFlag = flags;

    while (tmpFlag)
    {
        if (tmpFlag & CL_MIGRATE_MEM_OBJECT_HOST)
        {
            ss << "CL_MIGRATE_MEM_OBJECT_HOST";
            tmpFlag &= ~CL_MIGRATE_MEM_OBJECT_HOST;
        }
        else if (tmpFlag & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)
        {
            ss << "CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED";
            tmpFlag &= ~CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED;
        }
        else
        {
            ss << StringUtils::ToString(tmpFlag);
            tmpFlag = 0;
        }

        if (tmpFlag != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetMapFlagsString(const cl_map_flags flags)
{
    if (flags == 0)
    {
        return "0";
    }

    std::ostringstream ss;
    cl_map_flags tmpFlags = flags;

    while (tmpFlags)
    {
        if (tmpFlags & CL_MAP_READ)
        {
            ss << "CL_MAP_READ";
            tmpFlags &= ~CL_MAP_READ;
        }
        else if (tmpFlags & CL_MAP_WRITE)
        {
            ss << "CL_MAP_WRITE";
            tmpFlags &= ~CL_MAP_WRITE;
        }
        else if (tmpFlags & CL_MAP_WRITE_INVALIDATE_REGION)
        {
            ss << "CL_MAP_WRITE_INVALIDATE_REGION";
            tmpFlags &= ~CL_MAP_WRITE_INVALIDATE_REGION;
        }
        else
        {
            ss << StringUtils::ToString(tmpFlags);
            tmpFlags = 0;
        }

        if (tmpFlags != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetBufferCreateString(
    const cl_buffer_create_type type)
{
    switch (type)
    {
            CASE(CL_BUFFER_CREATE_TYPE_REGION)

        default: return StringUtils::ToString(type);
    }
}

std::string CLStringUtils::GetBufferInfoString(
    const cl_buffer_create_type type, const void* info)
{
    std::ostringstream ss;

    ss << '[';

    if (type == CL_BUFFER_CREATE_TYPE_REGION)
    {
        const cl_buffer_region* region = (const cl_buffer_region*)info;

        if (region != NULL)
        {
            ss << '{' << region->origin << ',' << region->size << '}';
        }
        else
        {
            ss << "NULL";
        }
    }
    else
    {
        ss << StringUtils::ToHexString(info);
    }

    ss << ']';
    return ss.str();
}

std::string CLStringUtils::GetChannelOrderString(const cl_channel_order order)
{
    switch (order)
    {
            CASE(CL_R);
            CASE(CL_A);
            CASE(CL_RG);
            CASE(CL_RA);
            CASE(CL_RGB);
            CASE(CL_RGBA);
            CASE(CL_BGRA);
            CASE(CL_ARGB);
            CASE(CL_INTENSITY);
            CASE(CL_LUMINANCE);
            CASE(CL_Rx);
            CASE(CL_RGx);
            CASE(CL_RGBx);
            CASE(CL_DEPTH);
            CASE(CL_DEPTH_STENCIL);
            CASE(CL_sRGB);
            CASE(CL_sRGBx);
            CASE(CL_sRGBA);
            CASE(CL_sBGRA);
            CASE(CL_ABGR);

        default: return StringUtils::ToString(order);
    }
}

std::string CLStringUtils::GetChannelTypeString(const cl_channel_type type)
{
    switch (type)
    {
            CASE(CL_SNORM_INT8);
            CASE(CL_SNORM_INT16);
            CASE(CL_UNORM_INT8);
            CASE(CL_UNORM_INT16);
            CASE(CL_UNORM_SHORT_565);
            CASE(CL_UNORM_SHORT_555);
            CASE(CL_UNORM_INT_101010);
            CASE(CL_SIGNED_INT8);
            CASE(CL_SIGNED_INT16);
            CASE(CL_SIGNED_INT32);
            CASE(CL_UNSIGNED_INT8);
            CASE(CL_UNSIGNED_INT16);
            CASE(CL_UNSIGNED_INT32);
            CASE(CL_HALF_FLOAT);
            CASE(CL_FLOAT);
            CASE(CL_UNORM_INT24);

        default: return StringUtils::ToString(type);
    }
}

std::string CLStringUtils::GetImageFormatsString(const cl_image_format* format, size_t num_entries, bool include_brackets)
{
    if (format == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    while (num_entries > 0 && format != NULL)
    {
        ss << '{' << GetChannelOrderString(format->image_channel_order) << ',';
        ss << GetChannelTypeString(format->image_channel_data_type) << '}';

        if (--num_entries > 0)
        {
            ss << ',';
            format++;
        }
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetAddressingModeString(const cl_addressing_mode mode)
{
    switch (mode)
    {
            CASE(CL_ADDRESS_NONE);
            CASE(CL_ADDRESS_CLAMP_TO_EDGE);
            CASE(CL_ADDRESS_CLAMP);
            CASE(CL_ADDRESS_REPEAT);
            CASE(CL_ADDRESS_MIRRORED_REPEAT);

        default: return StringUtils::ToString(mode);
    }
}

std::string CLStringUtils::GetFilterModeString(const cl_filter_mode mode)
{
    switch (mode)
    {
            CASE(CL_FILTER_NEAREST);
            CASE(CL_FILTER_LINEAR);

        default: return StringUtils::ToString(mode);
    }
}

std::string CLStringUtils::GetSamplerInfoString(const cl_sampler_info param_name)
{
    switch (param_name)
    {
            CASE(CL_SAMPLER_REFERENCE_COUNT);
            CASE(CL_SAMPLER_CONTEXT);
            CASE(CL_SAMPLER_NORMALIZED_COORDS);
            CASE(CL_SAMPLER_ADDRESSING_MODE);
            CASE(CL_SAMPLER_FILTER_MODE);
            CASE(CL_SAMPLER_MIP_FILTER_MODE);
            CASE(CL_SAMPLER_LOD_MIN);
            CASE(CL_SAMPLER_LOD_MAX);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetSamplerInfoValueString(const cl_sampler_info param_name, const void* param_value, cl_int ret_val, bool include_brackets)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;

        if (include_brackets)
        {
            ss << '[';
        }

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_SAMPLER_REFERENCE_COUNT:
                    ss << *((cl_uint*)param_value);
                    break;

                case CL_SAMPLER_CONTEXT:
                    ss << StringUtils::ToHexString(*((cl_context*)param_value));
                    break;

                case CL_SAMPLER_NORMALIZED_COORDS:
                {
                    cl_bool* val = (cl_bool*)param_value;
                    ss << GetBoolString(*val);
                    break;
                }

                case CL_SAMPLER_ADDRESSING_MODE:
                    ss << GetAddressingModeString(*((cl_addressing_mode*)param_value));
                    break;

                case CL_SAMPLER_FILTER_MODE:
                case CL_SAMPLER_MIP_FILTER_MODE:
                    ss << GetFilterModeString(*((cl_filter_mode*)param_value));
                    break;

                case CL_SAMPLER_LOD_MIN:
                case CL_SAMPLER_LOD_MAX:
                    ss << *((float*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        if (include_brackets)
        {
            ss << ']';
        }

        return ss.str();
    }
    else
    {
        return "NULL";
    }
}


std::string CLStringUtils::GetSamplerPropertiesString(const cl_sampler_properties* pProperties, std::vector<cl_sampler_properties> properties, bool include_brackets)
{
    if (pProperties == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << '{';

    int num_properties = 0;

    for (std::vector<cl_sampler_properties>::const_iterator it = properties.begin(); it != properties.end() && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES; it++)
    {
        cl_sampler_info samplerInfo = (cl_sampler_info)(*it);
        ss << GetSamplerInfoString(samplerInfo) << ',';
        num_properties++;

        if (++it == properties.end())
        {
            break;
        }
        else
        {
            const void* param_value = (const void*)*it;
            ss << CLStringUtils::GetSamplerInfoValueString(samplerInfo, &param_value, CL_SUCCESS, false) << ",";
            num_properties++;
        }
    }

    if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
    {
        ss << "...}";
    }
    else
    {
        ss << "NULL}";
    }

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();

}

std::string CLStringUtils::GetDeviceTypeString(const cl_device_type type)
{
    if (type == CL_DEVICE_TYPE_ALL)
    {
        return "CL_DEVICE_TYPE_ALL";
    }
    else if (type == 0)
    {
        return "0";
    }

    std::ostringstream ss;
    cl_device_type t = type;

    while (t)
    {
        if (t & CL_DEVICE_TYPE_DEFAULT)
        {
            ss << "CL_DEVICE_TYPE_DEFAULT";
            t &= ~CL_DEVICE_TYPE_DEFAULT;
        }
        else if (t & CL_DEVICE_TYPE_CPU)
        {
            ss << "CL_DEVICE_TYPE_CPU";
            t &= ~CL_DEVICE_TYPE_CPU;
        }
        else if (t & CL_DEVICE_TYPE_GPU)
        {
            ss << "CL_DEVICE_TYPE_GPU";
            t &= ~CL_DEVICE_TYPE_GPU;
        }
        else if (t & CL_DEVICE_TYPE_ACCELERATOR)
        {
            ss << "CL_DEVICE_TYPE_ACCELERATOR";
            t &= ~CL_DEVICE_TYPE_ACCELERATOR;
        }
        else if (t & CL_DEVICE_TYPE_CUSTOM)
        {
            ss << "CL_DEVICE_TYPE_CUSTOM";
            t &= ~CL_DEVICE_TYPE_CUSTOM;
        }
        else if (t & CL_HSA_ENABLED_AMD)
        {
            ss << "CL_HSA_ENABLED_AMD";
            t &= ~CL_HSA_ENABLED_AMD;
        }
        else if (t & CL_HSA_DISABLED_AMD)
        {
            ss << "CL_HSA_DISABLED_AMD";
            t &= ~CL_HSA_DISABLED_AMD;
        }
        else
        {
            ss << StringUtils::ToString(t);
            t = 0;
        }

        if (t != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetPlatformInfoString(const cl_platform_info param_name)
{
    switch (param_name)
    {
            CASE(CL_PLATFORM_PROFILE);
            CASE(CL_PLATFORM_VERSION);
            CASE(CL_PLATFORM_NAME);
            CASE(CL_PLATFORM_VENDOR);
            CASE(CL_PLATFORM_EXTENSIONS);
            CASE(CL_PLATFORM_ICD_SUFFIX_KHR);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetPlatformInfoValueString(const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            ss << GetStringString((char*)param_value, false);
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}


std::string CLStringUtils::GetDeviceInfoString(const cl_device_info param_name)
{
    switch (param_name)
    {
            CASE(CL_DEVICE_TYPE);
            CASE(CL_DEVICE_VENDOR_ID);
            CASE(CL_DEVICE_MAX_COMPUTE_UNITS);
            CASE(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            CASE(CL_DEVICE_MAX_WORK_GROUP_SIZE);
            CASE(CL_DEVICE_MAX_WORK_ITEM_SIZES);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
            CASE(CL_DEVICE_MAX_CLOCK_FREQUENCY);
            CASE(CL_DEVICE_ADDRESS_BITS);
            CASE(CL_DEVICE_MAX_READ_IMAGE_ARGS);
            CASE(CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
            CASE(CL_DEVICE_MAX_MEM_ALLOC_SIZE);
            CASE(CL_DEVICE_IMAGE2D_MAX_WIDTH);
            CASE(CL_DEVICE_IMAGE2D_MAX_HEIGHT);
            CASE(CL_DEVICE_IMAGE3D_MAX_WIDTH);
            CASE(CL_DEVICE_IMAGE3D_MAX_HEIGHT);
            CASE(CL_DEVICE_IMAGE3D_MAX_DEPTH);
            CASE(CL_DEVICE_IMAGE_SUPPORT);
            CASE(CL_DEVICE_MAX_PARAMETER_SIZE);
            CASE(CL_DEVICE_MAX_SAMPLERS);
            CASE(CL_DEVICE_MEM_BASE_ADDR_ALIGN);
            CASE(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
            CASE(CL_DEVICE_SINGLE_FP_CONFIG);
            CASE(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
            CASE(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
            CASE(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
            CASE(CL_DEVICE_GLOBAL_MEM_SIZE);
            CASE(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
            CASE(CL_DEVICE_MAX_CONSTANT_ARGS);
            CASE(CL_DEVICE_LOCAL_MEM_TYPE);
            CASE(CL_DEVICE_LOCAL_MEM_SIZE);
            CASE(CL_DEVICE_ERROR_CORRECTION_SUPPORT);
            CASE(CL_DEVICE_PROFILING_TIMER_RESOLUTION);
            CASE(CL_DEVICE_ENDIAN_LITTLE);
            CASE(CL_DEVICE_AVAILABLE);
            CASE(CL_DEVICE_COMPILER_AVAILABLE);
            CASE(CL_DEVICE_EXECUTION_CAPABILITIES);
            //CASE(CL_DEVICE_QUEUE_PROPERTIES); // this is deprecated in OCL 2.0 and CL_DEVICE_QUEUE_ON_HOST_PROPERTIES has replaced it (same ordinal value)
            CASE(CL_DEVICE_QUEUE_ON_HOST_PROPERTIES);
            CASE(CL_DEVICE_NAME);
            CASE(CL_DEVICE_VENDOR);
            CASE(CL_DRIVER_VERSION);
            CASE(CL_DEVICE_PROFILE);
            CASE(CL_DEVICE_VERSION);
            CASE(CL_DEVICE_EXTENSIONS);
            CASE(CL_DEVICE_PLATFORM);
            CASE(CL_DEVICE_DOUBLE_FP_CONFIG);
            CASE(CL_DEVICE_HALF_FP_CONFIG);
            CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
            CASE(CL_DEVICE_HOST_UNIFIED_MEMORY);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
            CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
            CASE(CL_DEVICE_OPENCL_C_VERSION);
            CASE(CL_DEVICE_LINKER_AVAILABLE);
            CASE(CL_DEVICE_BUILT_IN_KERNELS);
            CASE(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
            CASE(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
            CASE(CL_DEVICE_PARENT_DEVICE);
            CASE(CL_DEVICE_PARTITION_MAX_SUB_DEVICES);
            CASE(CL_DEVICE_PARTITION_PROPERTIES);
            CASE(CL_DEVICE_PARTITION_AFFINITY_DOMAIN);
            CASE(CL_DEVICE_PARTITION_TYPE);
            CASE(CL_DEVICE_REFERENCE_COUNT);
            CASE(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC);
            CASE(CL_DEVICE_PRINTF_BUFFER_SIZE);
            CASE(CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
            CASE(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
            CASE(CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
            CASE(CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE);
            CASE(CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES);
            CASE(CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE);
            CASE(CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE);
            CASE(CL_DEVICE_MAX_ON_DEVICE_QUEUES);
            CASE(CL_DEVICE_MAX_ON_DEVICE_EVENTS);
            CASE(CL_DEVICE_SVM_CAPABILITIES);
            CASE(CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE);
            CASE(CL_DEVICE_MAX_PIPE_ARGS);
            CASE(CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS);
            CASE(CL_DEVICE_PIPE_MAX_PACKET_SIZE);
            CASE(CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT);
            CASE(CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT);
            CASE(CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT);
            CASE(CL_DEVICE_PARENT_DEVICE_EXT);
            CASE(CL_DEVICE_PARTITION_TYPES_EXT);
            CASE(CL_DEVICE_AFFINITY_DOMAINS_EXT);
            CASE(CL_DEVICE_REFERENCE_COUNT_EXT);
            CASE(CL_DEVICE_PARTITION_STYLE_EXT);
            CASE(CL_DEVICE_PROFILING_TIMER_OFFSET_AMD);
            CASE(CL_DEVICE_TOPOLOGY_AMD);
            CASE(CL_DEVICE_BOARD_NAME_AMD);
            CASE(CL_DEVICE_GLOBAL_FREE_MEMORY_AMD);
            CASE(CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD);
            CASE(CL_DEVICE_SIMD_WIDTH_AMD);
            CASE(CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD);
            CASE(CL_DEVICE_WAVEFRONT_WIDTH_AMD);
            CASE(CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD);
            CASE(CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD);
            CASE(CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD);
            CASE(CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD);
            CASE(CL_DEVICE_LOCAL_MEM_BANKS_AMD);
            CASE(CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetContextInfoString(const cl_context_info param_name)
{
    switch (param_name)
    {
            CASE(CL_CONTEXT_REFERENCE_COUNT);
            CASE(CL_CONTEXT_DEVICES);
            CASE(CL_CONTEXT_PROPERTIES);
            CASE(CL_CONTEXT_NUM_DEVICES);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetContextInfoValueString(const cl_context_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_CONTEXT_REFERENCE_COUNT:
                case CL_CONTEXT_NUM_DEVICES:
                {
                    ss << *((cl_uint*)param_value);
                    break;
                }

                case CL_CONTEXT_DEVICES:
                {
                    cl_device_id* dids = (cl_device_id*)param_value;
                    size_t num_devices = param_value_size / sizeof(cl_device_id);

                    for (size_t i = 0; i < num_devices; i++)
                    {
                        ss << StringUtils::ToHexString(dids[i]);

                        if (i != num_devices - 1)
                        {
                            ss << ",";
                        }
                    }

                    break;
                }

                case CL_CONTEXT_PROPERTIES:
                {
                    std::vector<cl_context_properties> tmp;
                    cl_context_properties* props = (cl_context_properties*)param_value;
                    size_t num_properties = param_value_size / sizeof(cl_context_properties);

                    for (size_t i = 0; i < num_properties; i++)
                    {
                        // props is 0 terminated
                        if (props[i] != 0)
                        {
                            tmp.push_back(props[i]);
                        }
                    }

                    ss << GetContextPropertiesString(props, tmp, false);
                    break;
                }

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';

        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetCommandQueueInfoString(const cl_command_queue_info param_name)
{
    switch (param_name)
    {
            CASE(CL_QUEUE_CONTEXT);
            CASE(CL_QUEUE_DEVICE);
            CASE(CL_QUEUE_REFERENCE_COUNT);
            CASE(CL_QUEUE_PROPERTIES);
            CASE(CL_QUEUE_SIZE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetCommandQueueInfoValueString(const cl_command_queue_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_QUEUE_CONTEXT:
                    ss << StringUtils::ToHexString(*((cl_context*)param_value));
                    break;

                case CL_QUEUE_DEVICE:
                    ss << StringUtils::ToHexString(*((cl_device_id*)param_value));
                    break;

                case CL_QUEUE_REFERENCE_COUNT:
                case CL_QUEUE_SIZE:
                    ss << *((cl_uint*)param_value);
                    break;

                case CL_QUEUE_PROPERTIES:
                    ss << GetCommandQueuePropertyString(*((cl_command_queue_properties*)param_value));
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetProgramInfoString(const cl_program_info param_name)
{
    switch (param_name)
    {
            CASE(CL_PROGRAM_REFERENCE_COUNT);
            CASE(CL_PROGRAM_CONTEXT);
            CASE(CL_PROGRAM_NUM_DEVICES);
            CASE(CL_PROGRAM_DEVICES);
            CASE(CL_PROGRAM_SOURCE);
            CASE(CL_PROGRAM_BINARY_SIZES);
            CASE(CL_PROGRAM_BINARIES);
            CASE(CL_PROGRAM_NUM_KERNELS);
            CASE(CL_PROGRAM_KERNEL_NAMES);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProgramInfoValueString(const cl_program_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_PROGRAM_REFERENCE_COUNT:
                case CL_PROGRAM_NUM_DEVICES:
                case CL_PROGRAM_NUM_KERNELS:
                    ss << *((cl_uint*)param_value);
                    break;

                case CL_PROGRAM_CONTEXT:
                    ss << StringUtils::ToHexString(*((cl_context*)param_value));
                    break;

                case CL_PROGRAM_DEVICES:
                {
                    cl_device_id* dids = (cl_device_id*)param_value;
                    size_t num_devices = param_value_size / sizeof(cl_device_id);

                    for (size_t i = 0; i < num_devices; i++)
                    {
                        ss << StringUtils::ToHexString(dids[i]);

                        if (i != num_devices - 1)
                        {
                            ss << ",";
                        }
                    }

                    break;
                }

                case CL_PROGRAM_BINARY_SIZES:
                {
                    size_t* sizes = (size_t*)param_value;
                    size_t num_sizes = param_value_size / sizeof(size_t);

                    ss << GetSizeListString(sizes, num_sizes, false);

                    break;
                }

                case CL_PROGRAM_BINARIES:
                {
                    unsigned char** binary = (unsigned char**)param_value;
                    size_t num_binaries = param_value_size / sizeof(unsigned char**);

                    for (size_t i = 0; i < num_binaries; i++)
                    {
                        //show the pointer values of the binaries
                        ss << StringUtils::ToHexString((int*)binary[i]);

                        if (i != num_binaries - 1)
                        {
                            ss << ",";
                        }
                    }

                    break;
                }

                case CL_PROGRAM_KERNEL_NAMES:
                    ss << GetStringString((char*)param_value, false);
                    break;

                case CL_PROGRAM_SOURCE:
                    ss << GetStringString((char*)param_value, true);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetKernelInfoString(const cl_kernel_info param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_FUNCTION_NAME);
            CASE(CL_KERNEL_NUM_ARGS);
            CASE(CL_KERNEL_REFERENCE_COUNT);
            CASE(CL_KERNEL_CONTEXT);
            CASE(CL_KERNEL_PROGRAM);
            CASE(CL_KERNEL_ATTRIBUTES);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetKernelInfoValueString(const cl_kernel_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_KERNEL_FUNCTION_NAME:
                case CL_KERNEL_ATTRIBUTES:
                    ss << GetStringString((char*)param_value, false);
                    break;

                case CL_KERNEL_NUM_ARGS:
                case CL_KERNEL_REFERENCE_COUNT:
                    ss << *((cl_uint*)param_value);
                    break;

                case CL_KERNEL_CONTEXT:
                    ss << StringUtils::ToHexString(*((cl_context*)param_value));
                    break;

                case CL_KERNEL_PROGRAM:
                    ss << StringUtils::ToHexString(*((cl_program*)param_value));
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetKernelArgInfoString(const cl_kernel_arg_info param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_ARG_ADDRESS_QUALIFIER);
            CASE(CL_KERNEL_ARG_ACCESS_QUALIFIER);
            CASE(CL_KERNEL_ARG_TYPE_NAME);
            CASE(CL_KERNEL_ARG_TYPE_QUALIFIER);
            CASE(CL_KERNEL_ARG_NAME);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetKernelArgInfoValueString(const cl_kernel_arg_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
                    ss << GetKernelArgAddressQualifierString(*((cl_kernel_arg_address_qualifier*)param_value));
                    break;

                case CL_KERNEL_ARG_ACCESS_QUALIFIER:
                    ss << GetKernelArgAccessQualifierString(*((cl_kernel_arg_access_qualifier*)param_value));
                    break;

                case CL_KERNEL_ARG_TYPE_QUALIFIER:
                    ss << GetKernelArgTypeQualifierString(*((cl_kernel_arg_type_qualifier*)param_value));
                    break;

                case CL_KERNEL_ARG_TYPE_NAME:
                case CL_KERNEL_ARG_NAME:
                    ss << GetStringString((char*)param_value, false);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetKernelArgAddressQualifierString(const cl_kernel_arg_address_qualifier param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_ARG_ADDRESS_GLOBAL);
            CASE(CL_KERNEL_ARG_ADDRESS_LOCAL);
            CASE(CL_KERNEL_ARG_ADDRESS_CONSTANT);
            CASE(CL_KERNEL_ARG_ADDRESS_PRIVATE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetKernelArgAccessQualifierString(const cl_kernel_arg_access_qualifier param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_ARG_ACCESS_READ_ONLY);
            CASE(CL_KERNEL_ARG_ACCESS_WRITE_ONLY);
            CASE(CL_KERNEL_ARG_ACCESS_READ_WRITE);
            CASE(CL_KERNEL_ARG_ACCESS_NONE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetKernelArgTypeQualifierString(const cl_kernel_arg_type_qualifier type)
{
    if (type == 0)
    {
        return "CL_KERNEL_ARG_TYPE_NONE";
    }

    std::ostringstream ss;
    cl_kernel_arg_type_qualifier type_qualifier = type;

    while (type_qualifier)
    {
        if (type_qualifier & CL_KERNEL_ARG_TYPE_CONST)
        {
            ss << "CL_KERNEL_ARG_TYPE_CONST";
            type_qualifier &= ~CL_KERNEL_ARG_TYPE_CONST;
        }
        else if (type_qualifier & CL_KERNEL_ARG_TYPE_RESTRICT)
        {
            ss << "CL_KERNEL_ARG_TYPE_RESTRICT";
            type_qualifier &= ~CL_KERNEL_ARG_TYPE_RESTRICT;
        }
        else if (type_qualifier & CL_KERNEL_ARG_TYPE_VOLATILE)
        {
            ss << "CL_KERNEL_ARG_TYPE_VOLATILE";
            type_qualifier &= ~CL_KERNEL_ARG_TYPE_VOLATILE;
        }
        else if (type_qualifier & CL_KERNEL_ARG_TYPE_PIPE)
        {
            ss << "CL_KERNEL_ARG_TYPE_PIPE";
            type_qualifier &= ~CL_KERNEL_ARG_TYPE_PIPE;
        }
        else
        {
            ss << StringUtils::ToString(type_qualifier);
            type_qualifier = 0;
        }

        if (type_qualifier != 0)
        {
            ss << '|';
        }
    }

    return ss.str();

}

std::string CLStringUtils::GetKernelWorkGroupInfoString(const cl_kernel_work_group_info param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_WORK_GROUP_SIZE);
            CASE(CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
            CASE(CL_KERNEL_LOCAL_MEM_SIZE);
            CASE(CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
            CASE(CL_KERNEL_PRIVATE_MEM_SIZE);
            CASE(CL_KERNEL_GLOBAL_WORK_SIZE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProgramBuildInfoString(const cl_program_build_info param_name)
{
    switch (param_name)
    {
            CASE(CL_PROGRAM_BUILD_STATUS);
            CASE(CL_PROGRAM_BUILD_OPTIONS);
            CASE(CL_PROGRAM_BUILD_LOG);
            CASE(CL_PROGRAM_BINARY_TYPE);
            CASE(CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProgramBuildInfoValueString(const cl_program_build_info param_name, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_PROGRAM_BUILD_STATUS:
                    ss << GetBuildStatusString(*((cl_build_status*)param_value));
                    break;

                case CL_PROGRAM_BINARY_TYPE:
                    ss << GetProgramBinaryTypeString(*((cl_program_binary_type*)param_value));
                    break;

                case CL_PROGRAM_BUILD_OPTIONS:
                case CL_PROGRAM_BUILD_LOG:
                    ss << GetStringString((char*)param_value, true);
                    break;

                case CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE:
                    ss << *((size_t*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetBuildStatusString(const cl_build_status param_name)
{
    switch (param_name)
    {
            CASE(CL_BUILD_SUCCESS);
            CASE(CL_BUILD_NONE);
            CASE(CL_BUILD_ERROR);
            CASE(CL_BUILD_IN_PROGRESS);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProgramBinaryTypeString(const cl_program_binary_type param_name)
{
    switch (param_name)
    {
            CASE(CL_PROGRAM_BINARY_TYPE_NONE);
            CASE(CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
            CASE(CL_PROGRAM_BINARY_TYPE_LIBRARY);
            CASE(CL_PROGRAM_BINARY_TYPE_EXECUTABLE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetEventInfoString(const cl_event_info param_name)
{
    switch (param_name)
    {
            CASE(CL_EVENT_COMMAND_QUEUE);
            CASE(CL_EVENT_COMMAND_TYPE);
            CASE(CL_EVENT_REFERENCE_COUNT);
            CASE(CL_EVENT_COMMAND_EXECUTION_STATUS);
            CASE(CL_EVENT_CONTEXT);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProfilingInfoString(const cl_profiling_info param_name)
{
    switch (param_name)
    {
            CASE(CL_PROFILING_COMMAND_QUEUED);
            CASE(CL_PROFILING_COMMAND_SUBMIT);
            CASE(CL_PROFILING_COMMAND_START);
            CASE(CL_PROFILING_COMMAND_END);
            CASE(CL_PROFILING_COMMAND_COMPLETE);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetProfilingInfoValueString(const cl_profiling_info param_name, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        if (ret_value == CL_SUCCESS)
        {
            std::ostringstream ss;

            switch (param_name)
            {
                case CL_PROFILING_COMMAND_QUEUED:
                case CL_PROFILING_COMMAND_SUBMIT:
                case CL_PROFILING_COMMAND_START:
                case CL_PROFILING_COMMAND_END:
                case CL_PROFILING_COMMAND_COMPLETE:
                    ss << '[' << *(cl_ulong*)(param_value) << ']';
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }

            return ss.str();
        }
        else
        {
            return "[]";
        }
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetExecutionStatusString(const cl_int param_name)
{
    switch (param_name)
    {
            CASE(CL_COMPLETE);
            CASE(CL_RUNNING);
            CASE(CL_SUBMITTED);
            CASE(CL_QUEUED);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetQuotedString(const std::string& strInput, const char* src)
{
    if (src == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;
    ss << '"' << strInput << '"' ;
    return ss.str();
}

std::string CLStringUtils::GetStringString(const char* src, bool truncate)
{
    if (src == NULL)
    {
        return "NULL";
    }

    std::string str(src);

    if (truncate && str.length() > 60)
    {
        str = str.substr(0, 60).append("...");
    }

    size_t found = str.find_first_of("\n\r\t\"");

    while (std::string::npos != found)
    {
        char subst[] = { '\\', '\0', '\0' };

        switch (str[found])
        {
            case '\n': subst[1] = 'n'; break;

            case '\r': subst[1] = 'r'; break;

            case '\t': subst[1] = 't'; break;

            case '\"': subst[1] = '\"'; break;

            default: ++found; continue;
        }

        str.replace(found, 1, subst);
        found += 2;
        found = str.find_first_of("\n\r\t\"", found);
    }

    str.insert(0, 1, '\"').append(1, '\"');
    return str;
}

std::string CLStringUtils::GetProgramSourceString(
    const char** strings, const size_t* lengths, const cl_uint count)
{
    if (strings == NULL)
    {
        return "NULL";
    }

    if (count == 0)
    {
        return "[]";
    }

    std::ostringstream ss;
    ss << '[';

    for (cl_uint i = 0; i < count; ++i)
    {
        std::string src;

        if (lengths != NULL && lengths[i] != 0)
        {
            src = std::string(strings[i], lengths[i]);
        }
        else
        {
            src = strings[i];
        }

        if (i != 0)
        {
            ss << ',';
        }

        ss << GetStringString(src.c_str());
    }

    ss << ']';
    return ss.str();
}

std::string CLStringUtils::GetCLAPINameString(const CL_FUNC_TYPE type)
{
    switch (type)
    {
        case CL_FUNC_TYPE_clGetPlatformIDs:
            return std::string("clGetPlatformIDs");

        case CL_FUNC_TYPE_clGetPlatformInfo:
            return std::string("clGetPlatformInfo");

        case CL_FUNC_TYPE_clGetDeviceIDs:
            return std::string("clGetDeviceIDs");

        case CL_FUNC_TYPE_clGetDeviceInfo:
            return std::string("clGetDeviceInfo");

        case CL_FUNC_TYPE_clCreateContext:
            return std::string("clCreateContext");

        case CL_FUNC_TYPE_clCreateContextFromType:
            return std::string("clCreateContextFromType");

        case CL_FUNC_TYPE_clRetainContext:
            return std::string("clRetainContext");

        case CL_FUNC_TYPE_clReleaseContext:
            return std::string("clReleaseContext");

        case CL_FUNC_TYPE_clGetContextInfo:
            return std::string("clGetContextInfo");

        case CL_FUNC_TYPE_clCreateCommandQueue:
            return std::string("clCreateCommandQueue");

        case CL_FUNC_TYPE_clRetainCommandQueue:
            return std::string("clRetainCommandQueue");

        case CL_FUNC_TYPE_clReleaseCommandQueue:
            return std::string("clReleaseCommandQueue");

        case CL_FUNC_TYPE_clGetCommandQueueInfo:
            return std::string("clGetCommandQueueInfo");

        case CL_FUNC_TYPE_clSetCommandQueueProperty:
            return std::string("clSetCommandQueueProperty");

        case CL_FUNC_TYPE_clCreateBuffer:
            return std::string("clCreateBuffer");

        case CL_FUNC_TYPE_clCreateSubBuffer:
            return std::string("clCreateSubBuffer");

        case CL_FUNC_TYPE_clCreateImage2D:
            return std::string("clCreateImage2D");

        case CL_FUNC_TYPE_clCreateImage3D:
            return std::string("clCreateImage3D");

        case CL_FUNC_TYPE_clRetainMemObject:
            return std::string("clRetainMemObject");

        case CL_FUNC_TYPE_clReleaseMemObject:
            return std::string("clReleaseMemObject");

        case CL_FUNC_TYPE_clGetSupportedImageFormats:
            return std::string("clGetSupportedImageFormats");

        case CL_FUNC_TYPE_clGetMemObjectInfo:
            return std::string("clGetMemObjectInfo");

        case CL_FUNC_TYPE_clGetImageInfo:
            return std::string("clGetImageInfo");

        case CL_FUNC_TYPE_clSetMemObjectDestructorCallback:
            return std::string("clSetMemObjectDestructorCallback");

        case CL_FUNC_TYPE_clCreateSampler:
            return std::string("clCreateSampler");

        case CL_FUNC_TYPE_clRetainSampler:
            return std::string("clRetainSampler");

        case CL_FUNC_TYPE_clReleaseSampler:
            return std::string("clReleaseSampler");

        case CL_FUNC_TYPE_clGetSamplerInfo:
            return std::string("clGetSamplerInfo");

        case CL_FUNC_TYPE_clCreateProgramWithSource:
            return std::string("clCreateProgramWithSource");

        case CL_FUNC_TYPE_clCreateProgramWithBinary:
            return std::string("clCreateProgramWithBinary");

        case CL_FUNC_TYPE_clRetainProgram:
            return std::string("clRetainProgram");

        case CL_FUNC_TYPE_clReleaseProgram:
            return std::string("clReleaseProgram");

        case CL_FUNC_TYPE_clBuildProgram:
            return std::string("clBuildProgram");

        case CL_FUNC_TYPE_clUnloadCompiler:
            return std::string("clUnloadCompiler");

        case CL_FUNC_TYPE_clGetProgramInfo:
            return std::string("clGetProgramInfo");

        case CL_FUNC_TYPE_clGetProgramBuildInfo:
            return std::string("clGetProgramBuildInfo");

        case CL_FUNC_TYPE_clCreateKernel:
            return std::string("clCreateKernel");

        case CL_FUNC_TYPE_clCreateKernelsInProgram:
            return std::string("clCreateKernelsInProgram");

        case CL_FUNC_TYPE_clRetainKernel:
            return std::string("clRetainKernel");

        case CL_FUNC_TYPE_clReleaseKernel:
            return std::string("clReleaseKernel");

        case CL_FUNC_TYPE_clSetKernelArg:
            return std::string("clSetKernelArg");

        case CL_FUNC_TYPE_clGetKernelInfo:
            return std::string("clGetKernelInfo");

        case CL_FUNC_TYPE_clGetKernelWorkGroupInfo:
            return std::string("clGetKernelWorkGroupInfo");

        case CL_FUNC_TYPE_clWaitForEvents:
            return std::string("clWaitForEvents");

        case CL_FUNC_TYPE_clGetEventInfo:
            return std::string("clGetEventInfo");

        case CL_FUNC_TYPE_clCreateUserEvent:
            return std::string("clCreateUserEvent");

        case CL_FUNC_TYPE_clRetainEvent:
            return std::string("clRetainEvent");

        case CL_FUNC_TYPE_clReleaseEvent:
            return std::string("clReleaseEvent");

        case CL_FUNC_TYPE_clSetUserEventStatus:
            return std::string("clSetUserEventStatus");

        case CL_FUNC_TYPE_clSetEventCallback:
            return std::string("clSetEventCallback");

        case CL_FUNC_TYPE_clGetEventProfilingInfo:
            return std::string("clGetEventProfilingInfo");

        case CL_FUNC_TYPE_clFlush:
            return std::string("clFlush");

        case CL_FUNC_TYPE_clFinish:
            return std::string("clFinish");

        case CL_FUNC_TYPE_clEnqueueReadBuffer:
            return std::string("clEnqueueReadBuffer");

        case CL_FUNC_TYPE_clEnqueueReadBufferRect:
            return std::string("clEnqueueReadBufferRect");

        case CL_FUNC_TYPE_clEnqueueWriteBuffer:
            return std::string("clEnqueueWriteBuffer");

        case CL_FUNC_TYPE_clEnqueueWriteBufferRect:
            return std::string("clEnqueueWriteBufferRect");

        case CL_FUNC_TYPE_clEnqueueCopyBuffer:
            return std::string("clEnqueueCopyBuffer");

        case CL_FUNC_TYPE_clEnqueueCopyBufferRect:
            return std::string("clEnqueueCopyBufferRect");

        case CL_FUNC_TYPE_clEnqueueReadImage:
            return std::string("clEnqueueReadImage");

        case CL_FUNC_TYPE_clEnqueueWriteImage:
            return std::string("clEnqueueWriteImage");

        case CL_FUNC_TYPE_clEnqueueCopyImage:
            return std::string("clEnqueueCopyImage");

        case CL_FUNC_TYPE_clEnqueueCopyImageToBuffer:
            return std::string("clEnqueueCopyImageToBuffer");

        case CL_FUNC_TYPE_clEnqueueCopyBufferToImage:
            return std::string("clEnqueueCopyBufferToImage");

        case CL_FUNC_TYPE_clEnqueueMapBuffer:
            return std::string("clEnqueueMapBuffer");

        case CL_FUNC_TYPE_clEnqueueMapImage:
            return std::string("clEnqueueMapImage");

        case CL_FUNC_TYPE_clEnqueueUnmapMemObject:
            return std::string("clEnqueueUnmapMemObject");

        case CL_FUNC_TYPE_clEnqueueNDRangeKernel:
            return std::string("clEnqueueNDRangeKernel");

        case CL_FUNC_TYPE_clEnqueueTask:
            return std::string("clEnqueueTask");

        case CL_FUNC_TYPE_clEnqueueNativeKernel:
            return std::string("clEnqueueNativeKernel");

        case CL_FUNC_TYPE_clEnqueueMarker:
            return std::string("clEnqueueMarker");

        case CL_FUNC_TYPE_clEnqueueWaitForEvents:
            return std::string("clEnqueueWaitForEvents");

        case CL_FUNC_TYPE_clEnqueueBarrier:
            return std::string("clEnqueueBarrier");

        case CL_FUNC_TYPE_clCreateFromGLBuffer:
            return std::string("clCreateFromGLBuffer");

        case CL_FUNC_TYPE_clCreateFromGLTexture2D:
            return std::string("clCreateFromGLTexture2D");

        case CL_FUNC_TYPE_clCreateFromGLTexture3D:
            return std::string("clCreateFromGLTexture3D");

        case CL_FUNC_TYPE_clCreateFromGLRenderbuffer:
            return std::string("clCreateFromGLRenderbuffer");

        case CL_FUNC_TYPE_clGetGLObjectInfo:
            return std::string("clGetGLObjectInfo");

        case CL_FUNC_TYPE_clGetGLTextureInfo:
            return std::string("clGetGLTextureInfo");

        case CL_FUNC_TYPE_clEnqueueAcquireGLObjects:
            return std::string("clEnqueueAcquireGLObjects");

        case CL_FUNC_TYPE_clEnqueueReleaseGLObjects:
            return std::string("clEnqueueReleaseGLObjects");

        case CL_FUNC_TYPE_clCreateEventFromGLsyncKHR:
            return std::string("clCreateEventFromGLsyncKHR");

        case CL_FUNC_TYPE_clGetGLContextInfoKHR:
            return std::string("clGetGLContextInfoKHR");

        case CL_FUNC_TYPE_clCreateSubDevicesEXT:
            return std::string("clCreateSubDevicesEXT");

        case CL_FUNC_TYPE_clRetainDeviceEXT:
            return std::string("clRetainDeviceEXT");

        case CL_FUNC_TYPE_clReleaseDeviceEXT:
            return std::string("clReleaseDeviceEXT");

        case CL_FUNC_TYPE_clGetDeviceIDsFromD3D10KHR:
            return std::string("clGetDeviceIDsFromD3D10KHR");

        case CL_FUNC_TYPE_clCreateFromD3D10BufferKHR:
            return std::string("clCreateFromD3D10BufferKHR");

        case CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR:
            return std::string("clCreateFromD3D10Texture2DKHR");

        case CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR:
            return std::string("clCreateFromD3D10Texture3DKHR");

        case CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR:
            return std::string("clEnqueueAcquireD3D10ObjectsKHR");

        case CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR:
            return std::string("clEnqueueReleaseD3D10ObjectsKHR");

        case CL_FUNC_TYPE_clCreateSubDevices:
            return std::string("clCreateSubDevices");

        case CL_FUNC_TYPE_clRetainDevice:
            return std::string("clRetainDevice");

        case CL_FUNC_TYPE_clReleaseDevice:
            return std::string("clReleaseDevice");

        case CL_FUNC_TYPE_clCreateImage:
            return std::string("clCreateImage");

        case CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels:
            return std::string("clCreateProgramWithBuiltInKernels");

        case CL_FUNC_TYPE_clCompileProgram:
            return std::string("clCompileProgram");

        case CL_FUNC_TYPE_clLinkProgram:
            return std::string("clLinkProgram");

        case CL_FUNC_TYPE_clUnloadPlatformCompiler:
            return std::string("clUnloadPlatformCompiler");

        case CL_FUNC_TYPE_clGetKernelArgInfo:
            return std::string("clGetKernelArgInfo");

        case CL_FUNC_TYPE_clEnqueueFillBuffer:
            return std::string("clEnqueueFillBuffer");

        case CL_FUNC_TYPE_clEnqueueFillImage:
            return std::string("clEnqueueFillImage");

        case CL_FUNC_TYPE_clEnqueueMigrateMemObjects:
            return std::string("clEnqueueMigrateMemObjects");

        case CL_FUNC_TYPE_clEnqueueMarkerWithWaitList:
            return std::string("clEnqueueMarkerWithWaitList");

        case CL_FUNC_TYPE_clEnqueueBarrierWithWaitList:
            return std::string("clEnqueueBarrierWithWaitList");

        case CL_FUNC_TYPE_clGetExtensionFunctionAddressForPlatform:
            return std::string("clGetExtensionFunctionAddressForPlatform");

        case CL_FUNC_TYPE_clCreateFromGLTexture:
            return std::string("clCreateFromGLTexture");

        case CL_FUNC_TYPE_clGetExtensionFunctionAddress:
            return std::string("clGetExtensionFunctionAddress");

        case CL_FUNC_TYPE_clCreateCommandQueueWithProperties:
            return std::string("clCreateCommandQueueWithProperties");

        case CL_FUNC_TYPE_clCreateSamplerWithProperties:
            return std::string("clCreateSamplerWithProperties");

        case CL_FUNC_TYPE_clSVMAlloc:
            return std::string("clSVMAlloc");

        case CL_FUNC_TYPE_clSVMFree:
            return std::string("clSVMFree");

        case CL_FUNC_TYPE_clSetKernelArgSVMPointer:
            return std::string("clSetKernelArgSVMPointer");

        case CL_FUNC_TYPE_clSetKernelExecInfo:
            return std::string("clSetKernelExecInfo");

        case CL_FUNC_TYPE_clEnqueueSVMFree:
            return std::string("clEnqueueSVMFree");

        case CL_FUNC_TYPE_clEnqueueSVMMemcpy:
            return std::string("clEnqueueSVMMemcpy");

        case CL_FUNC_TYPE_clEnqueueSVMMemFill:
            return std::string("clEnqueueSVMMemFill");

        case CL_FUNC_TYPE_clEnqueueSVMMap:
            return std::string("clEnqueueSVMMap");

        case CL_FUNC_TYPE_clEnqueueSVMUnmap:
            return std::string("clEnqueueSVMUnmap");

        case CL_FUNC_TYPE_clCreatePipe:
            return std::string("clCreatePipe");

        case CL_FUNC_TYPE_clGetPipeInfo:
            return std::string("clGetPipeInfo");

        case CL_FUNC_TYPE_clSVMAllocAMD:
            return std::string("clSVMAllocAMD");

        case CL_FUNC_TYPE_clSVMFreeAMD:
            return std::string("clSVMFreeAMD");

        case CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD:
            return std::string("clSetKernelArgSVMPointerAMD");

        case CL_FUNC_TYPE_clSetKernelExecInfoAMD:
            return std::string("clSetKernelExecInfoAMD");

        case CL_FUNC_TYPE_clEnqueueSVMFreeAMD:
            return std::string("clEnqueueSVMFreeAMD");

        case CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD:
            return std::string("clEnqueueSVMMemcpyAMD");

        case CL_FUNC_TYPE_clEnqueueSVMMemFillAMD:
            return std::string("clEnqueueSVMMemFillAMD");

        case CL_FUNC_TYPE_clEnqueueSVMMapAMD:
            return std::string("clEnqueueSVMMapAMD");

        case CL_FUNC_TYPE_clEnqueueSVMUnmapAMD:
            return std::string("clEnqueueSVMUnmapAMD");

        default:
            return std::string("Unknown_function_type");
    }
}

std::string CLStringUtils::GetKernelWorkGroupInfoValueString(const cl_kernel_work_group_info param_name, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;

        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_KERNEL_WORK_GROUP_SIZE:
                    ss << *((size_t*)param_value);
                    break;

                case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
                case CL_KERNEL_GLOBAL_WORK_SIZE:
                    ss << "{";
                    ss << ((size_t*)param_value)[0] << ",";
                    ss << ((size_t*)param_value)[1] << ",";
                    ss << ((size_t*)param_value)[2];
                    ss << "}";
                    break;

                case CL_KERNEL_LOCAL_MEM_SIZE:
                    ss << *((cl_ulong*)param_value);
                    break;

                case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
                    ss << *((size_t*)param_value);
                    break;

                case CL_KERNEL_PRIVATE_MEM_SIZE:
                    ss << *((cl_ulong*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetCommandTypeString(const cl_command_type type)
{
    switch (type)
    {
            CASE(CL_COMMAND_NDRANGE_KERNEL);
            CASE(CL_COMMAND_TASK);
            CASE(CL_COMMAND_NATIVE_KERNEL);
            CASE(CL_COMMAND_READ_BUFFER);
            CASE(CL_COMMAND_WRITE_BUFFER);
            CASE(CL_COMMAND_COPY_BUFFER);
            CASE(CL_COMMAND_READ_IMAGE);
            CASE(CL_COMMAND_WRITE_IMAGE);
            CASE(CL_COMMAND_COPY_IMAGE);
            CASE(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
            CASE(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
            CASE(CL_COMMAND_MAP_BUFFER);
            CASE(CL_COMMAND_MAP_IMAGE);
            CASE(CL_COMMAND_UNMAP_MEM_OBJECT);
            CASE(CL_COMMAND_MARKER);
            CASE(CL_COMMAND_ACQUIRE_GL_OBJECTS);
            CASE(CL_COMMAND_RELEASE_GL_OBJECTS);
            CASE(CL_COMMAND_READ_BUFFER_RECT);
            CASE(CL_COMMAND_WRITE_BUFFER_RECT);
            CASE(CL_COMMAND_COPY_BUFFER_RECT);
            CASE(CL_COMMAND_USER);
            CASE(CL_COMMAND_BARRIER);
            CASE(CL_COMMAND_MIGRATE_MEM_OBJECTS);
            CASE(CL_COMMAND_FILL_BUFFER);
            CASE(CL_COMMAND_FILL_IMAGE);
            CASE(CL_COMMAND_SVM_FREE);
            CASE(CL_COMMAND_SVM_MEMCPY);
            CASE(CL_COMMAND_SVM_MEMFILL);
            CASE(CL_COMMAND_SVM_MAP);
            CASE(CL_COMMAND_SVM_UNMAP);
            CASE(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR);
#ifdef _WIN32
            CASE(CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR);
            CASE(CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR);
#endif
            CASE(CL_COMMAND_WAIT_SIGNAL_AMD);
            CASE(CL_COMMAND_WRITE_SIGNAL_AMD);
            CASE(CL_COMMAND_MAKE_BUFFERS_RESIDENT_AMD);

        default: return StringUtils::ToString(type);
    }
}

std::string CLStringUtils::GetEventInfoValueString(const cl_event_info param_name, const void* param_value, cl_int ret_value)
{
    if (param_value == NULL)
    {
        return  "NULL";
    }

    std::ostringstream ss;

    ss << '[';

    if (ret_value == CL_SUCCESS)
    {
        switch (param_name)
        {
            case CL_EVENT_COMMAND_TYPE:
                ss << GetCommandTypeString(*((cl_command_type*)param_value));
                break;

            case CL_EVENT_COMMAND_QUEUE:
                ss << StringUtils::ToHexString(*((cl_command_queue*)param_value));
                break;

            case CL_EVENT_CONTEXT:
                ss << StringUtils::ToHexString(*((cl_context*)param_value));
                break;

            case CL_EVENT_COMMAND_EXECUTION_STATUS:
                ss << GetExecutionStatusString(*((cl_int*)param_value));
                break;

            case CL_EVENT_REFERENCE_COUNT:
                ss << *((cl_uint*)param_value);
                break;

            default:
                ss << StringUtils::ToString(*((int*)param_value));
                break;
        }
    }

    ss << ']';

    return ss.str();
}

std::string CLStringUtils::GetEventListString(const cl_event* event_wait_list, const std::vector<cl_event>& vecEvents)
{
    if (event_wait_list == NULL)
    {
        return "NULL";
    }

    if (vecEvents.size() == 0)
    {
        return "[]";
    }

    std::ostringstream ss;
    ss << '[';

    for (size_t i = 0; i != vecEvents.size(); i++)
    {
        ss << StringUtils::ToHexString(vecEvents[i]);

        if (i != vecEvents.size() - 1)
        {
            ss << ',';
        }
    }

    ss << ']';
    return ss.str();
}

std::string CLStringUtils::GetEventString(const cl_event event)
{
    if (event == NULL)
    {
        return "NULL";
    }
    else
    {
        std::ostringstream ss;
        ss << '[' << StringUtils::ToHexString(event) << ']';
        return ss.str();
    }
}

std::string CLStringUtils::GetDeviceInfoValueString(const cl_device_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_DEVICE_TYPE                              :
                {
                    cl_device_type* type = (cl_device_type*)param_value;
                    ss << GetDeviceTypeString(*type);
                    break;
                }

                case CL_DEVICE_VENDOR_ID                         :
                case CL_DEVICE_MAX_COMPUTE_UNITS                 :
                case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS          :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR       :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT      :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT        :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG       :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT      :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE     :
                case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF       :

                case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR          :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT         :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT           :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG          :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT         :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE        :
                case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF          :

                case CL_DEVICE_MAX_CLOCK_FREQUENCY               :
                case CL_DEVICE_ADDRESS_BITS                      :
                case CL_DEVICE_MAX_READ_IMAGE_ARGS               :
                case CL_DEVICE_MAX_WRITE_IMAGE_ARGS              :
                case CL_DEVICE_MAX_SAMPLERS                      :
                case CL_DEVICE_MEM_BASE_ADDR_ALIGN               :
                case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE         :
                case CL_DEVICE_MAX_CONSTANT_ARGS                 :
                case CL_DEVICE_REFERENCE_COUNT_EXT               :
                case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE          :
                case CL_DEVICE_PARTITION_MAX_SUB_DEVICES         :
                case CL_DEVICE_REFERENCE_COUNT                   :

                case CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD         :
                case CL_DEVICE_SIMD_WIDTH_AMD                    :
                case CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD        :
                case CL_DEVICE_WAVEFRONT_WIDTH_AMD               :
                case CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD           :
                case CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD      :
                case CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD :
                case CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD:
                case CL_DEVICE_LOCAL_MEM_BANKS_AMD               :
                case CL_DEVICE_IMAGE_PITCH_ALIGNMENT             :
                case CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT      :
                {
                    cl_uint* val = (cl_uint*)param_value;
                    ss << StringUtils::ToString(*val);
                    break;
                }

                case CL_DEVICE_MAX_WORK_GROUP_SIZE               :
                case CL_DEVICE_IMAGE2D_MAX_WIDTH                 :
                case CL_DEVICE_IMAGE2D_MAX_HEIGHT                :
                case CL_DEVICE_IMAGE3D_MAX_WIDTH                 :
                case CL_DEVICE_IMAGE3D_MAX_HEIGHT                :
                case CL_DEVICE_IMAGE3D_MAX_DEPTH                 :
                case CL_DEVICE_MAX_PARAMETER_SIZE                :
                case CL_DEVICE_PROFILING_TIMER_RESOLUTION        :
                case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE             :
                case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE              :
                case CL_DEVICE_PRINTF_BUFFER_SIZE                :
                {
                    size_t* val = (size_t*)param_value;
                    ss << StringUtils::ToString(*val);
                    break;
                }

                case CL_DEVICE_MAX_MEM_ALLOC_SIZE                :
                case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE             :
                case CL_DEVICE_GLOBAL_MEM_SIZE                   :
                case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE          :
                case CL_DEVICE_LOCAL_MEM_SIZE                    :
                case CL_DEVICE_PROFILING_TIMER_OFFSET_AMD        :
                {
                    cl_ulong* val = (cl_ulong*)param_value;
                    ss << StringUtils::ToString(*val);
                    break;
                }

                case CL_DEVICE_IMAGE_SUPPORT                     :
                case CL_DEVICE_ERROR_CORRECTION_SUPPORT          :
                case CL_DEVICE_ENDIAN_LITTLE                     :
                case CL_DEVICE_AVAILABLE                         :
                case CL_DEVICE_COMPILER_AVAILABLE                :
                case CL_DEVICE_HOST_UNIFIED_MEMORY               :
                case CL_DEVICE_LINKER_AVAILABLE                  :
                case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC       :
                case CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD        :
                {
                    cl_bool* val = (cl_bool*)param_value;
                    ss << GetBoolString(*val);
                    break;
                }

                case CL_DEVICE_SINGLE_FP_CONFIG                  :
                case CL_DEVICE_DOUBLE_FP_CONFIG                  :
                case CL_DEVICE_HALF_FP_CONFIG                  :
                {
                    cl_device_fp_config fp_config = *((cl_device_fp_config*)param_value);

                    while (fp_config)
                    {
                        if (fp_config & CL_FP_DENORM)
                        {
                            ss << "CL_FP_DENORM";
                            fp_config &= ~CL_FP_DENORM;
                        }
                        else if (fp_config & CL_FP_INF_NAN)
                        {
                            ss << "CL_FP_INF_NAN";
                            fp_config &= ~CL_FP_INF_NAN;
                        }
                        else if (fp_config & CL_FP_ROUND_TO_NEAREST)
                        {
                            ss << "CL_FP_ROUND_TO_NEAREST";
                            fp_config &= ~CL_FP_ROUND_TO_NEAREST;
                        }
                        else if (fp_config & CL_FP_ROUND_TO_ZERO)
                        {
                            ss << "CL_FP_ROUND_TO_ZERO";
                            fp_config &= ~CL_FP_ROUND_TO_ZERO;
                        }
                        else if (fp_config & CL_FP_ROUND_TO_INF)
                        {
                            ss << "CL_FP_ROUND_TO_INF";
                            fp_config &= ~CL_FP_ROUND_TO_INF;
                        }
                        else if (fp_config & CL_FP_FMA)
                        {
                            ss << "CL_FP_FMA";
                            fp_config &= ~CL_FP_FMA;
                        }
                        else if (fp_config & CL_FP_SOFT_FLOAT)
                        {
                            ss << "CL_FP_SOFT_FLOAT";
                            fp_config &= ~CL_FP_SOFT_FLOAT;
                        }
                        else if (fp_config & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)
                        {
                            ss << "CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT";
                            fp_config &= ~CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT;
                        }
                        else
                        {
                            ss << StringUtils::ToString(fp_config);
                            fp_config = 0;
                        }

                        if (fp_config != 0)
                        {
                            ss << '|';
                        }
                    }

                    break;
                }

                case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE             :
                {
                    cl_device_mem_cache_type val = *((cl_device_mem_cache_type*)param_value);

                    switch (val)
                    {
                            CASESTR(CL_NONE);
                            CASESTR(CL_READ_ONLY_CACHE);
                            CASESTR(CL_READ_WRITE_CACHE);

                        default:
                            ss << StringUtils::ToString(val);
                            break;
                    }

                    break;
                }

                case CL_DEVICE_LOCAL_MEM_TYPE                    :
                {
                    cl_device_local_mem_type type = *((cl_device_local_mem_type*)param_value);

                    switch (type)
                    {
                            CASESTR(CL_LOCAL);
                            CASESTR(CL_GLOBAL);

                        default:
                            ss << StringUtils::ToString(type);
                            break;
                    }

                    break;
                }

                case CL_DEVICE_EXECUTION_CAPABILITIES            :
                {
                    cl_device_exec_capabilities cap = *((cl_device_exec_capabilities*)param_value);

                    while (cap)
                    {
                        if (cap & CL_EXEC_KERNEL)
                        {
                            ss << "CL_EXEC_KERNEL";
                            cap &= ~CL_EXEC_KERNEL;
                        }
                        else if (cap & CL_EXEC_NATIVE_KERNEL)
                        {
                            ss << "CL_EXEC_NATIVE_KERNEL";
                            cap &= ~CL_EXEC_NATIVE_KERNEL;
                        }
                        else
                        {
                            ss << StringUtils::ToString(cap);
                            cap = 0;
                        }

                        if (cap != 0)
                        {
                            ss << '|';
                        }
                    }

                    break;
                }

                case CL_DEVICE_QUEUE_PROPERTIES                  :
                {
                    cl_command_queue_properties props = *((cl_command_queue_properties*)param_value);
                    ss << GetCommandQueuePropertyString(props);
                    break;
                }

                case CL_DEVICE_NAME                              :
                case CL_DEVICE_VENDOR                            :
                case CL_DRIVER_VERSION                           :
                case CL_DEVICE_VERSION                           :
                case CL_DEVICE_PROFILE                           :
                case CL_DEVICE_EXTENSIONS                        :
                case CL_DEVICE_OPENCL_C_VERSION                  :
                case CL_DEVICE_BUILT_IN_KERNELS                  :
                case CL_DEVICE_BOARD_NAME_AMD                    :
                {
                    ss << GetStringString((char*)param_value, false);
                    break;
                }

                case CL_DEVICE_MAX_WORK_ITEM_SIZES               :
                case CL_DEVICE_GLOBAL_FREE_MEMORY_AMD            :
                {
                    size_t* sizes = (size_t*)param_value;
                    size_t num_sizes = param_value_size / sizeof(size_t);

                    for (size_t i = 0; i < num_sizes; i++)
                    {
                        ss << sizes[i];

                        if (i != num_sizes - 1)
                        {
                            ss << ",";
                        }
                    }

                    break;
                }

                case CL_DEVICE_PLATFORM:
                {
                    ss << StringUtils::ToHexString(*((cl_platform_id*)param_value));
                    break;
                }

                case CL_DEVICE_PARENT_DEVICE:
                case CL_DEVICE_PARENT_DEVICE_EXT:
                {
                    ss << StringUtils::ToHexString(*((cl_device_id*)param_value));
                    break;
                }

                case CL_DEVICE_PARTITION_PROPERTIES:
                case CL_DEVICE_PARTITION_TYPE:
                {
                    cl_device_partition_property* props = (cl_device_partition_property*)param_value;
                    size_t num_properties = param_value_size / sizeof(cl_device_partition_property);

                    for (size_t i = 0; i < num_properties; i++)
                    {
                        ss << GetPartitionPropertyString(props[i]);

                        if (i < num_properties - 1)
                        {
                            ss << ',';
                        }
                    }

                    break;
                }

                case CL_DEVICE_PARTITION_TYPES_EXT:
                {
                    cl_device_partition_property_ext* props = (cl_device_partition_property_ext*)param_value;
                    size_t num_properties = param_value_size / sizeof(cl_device_partition_property_ext);

                    for (size_t i = 0; i < num_properties; i++)
                    {
                        ss << GetPartitionPropertyExtString(props[i]);

                        if (i < num_properties - 1)
                        {
                            ss << ',';
                        }
                    }

                    break;
                }

                case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
                {
                    cl_device_affinity_domain affinity_domain = *(cl_device_affinity_domain*)param_value;
                    ss << GetPartitionAffinityDomainString(affinity_domain);
                    break;
                }

                case CL_DEVICE_AFFINITY_DOMAINS_EXT:
                {
                    cl_device_partition_property_ext* props = (cl_device_partition_property_ext*)param_value;
                    size_t num_properties = param_value_size / sizeof(cl_device_partition_property_ext);

                    for (size_t i = 0; i < num_properties; i++)
                    {
                        ss << GetPartitionAffinityDomainExtString(props[i]);

                        if (i < num_properties - 1)
                        {
                            ss << ',';
                        }
                    }

                    break;
                }

                case CL_DEVICE_PARTITION_STYLE_EXT:
                {
                    std::vector<cl_device_partition_property_ext> tmp;
                    cl_device_partition_property_ext* props = (cl_device_partition_property_ext*)param_value;
                    size_t num_properties = param_value_size / sizeof(cl_device_partition_property_ext);

                    cl_device_partition_property_ext sub_list_terminator = 0;

                    for (size_t i = 0; i < num_properties; i++)
                    {
                        if (i == 0 && props[i] == CL_DEVICE_PARTITION_BY_NAMES_EXT)
                        {
                            sub_list_terminator = CL_PARTITION_BY_NAMES_LIST_END_EXT;
                        }

                        if (props[i] != sub_list_terminator)
                        {
                            tmp.push_back(props[i]);
                        }
                    }

                    ss << GetPartitionPropertiesExtString(tmp, false);
                    break;
                }

                case CL_DEVICE_TOPOLOGY_AMD:
                {
                    cl_device_topology_amd* topology = (cl_device_topology_amd*)param_value;
                    ss << GetDeviceTopologyAMDString(topology, false);
                    break;
                }

                case CL_DEVICE_SVM_CAPABILITIES                  :
                {
                    cl_device_svm_capabilities cap = *((cl_device_svm_capabilities*)param_value);

                    while (cap)
                    {
                        if (cap & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
                        {
                            ss << "CL_DEVICE_SVM_COARSE_GRAIN_BUFFER";
                            cap &= ~CL_DEVICE_SVM_COARSE_GRAIN_BUFFER;
                        }
                        else if (cap & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
                        {
                            ss << "CL_DEVICE_SVM_FINE_GRAIN_BUFFER";
                            cap &= ~CL_DEVICE_SVM_FINE_GRAIN_BUFFER;
                        }
                        else if (cap & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
                        {
                            ss << "CL_DEVICE_SVM_FINE_GRAIN_SYSTEM";
                            cap &= ~CL_DEVICE_SVM_FINE_GRAIN_SYSTEM;
                        }
                        else if (cap & CL_DEVICE_SVM_ATOMICS)
                        {
                            ss << "CL_DEVICE_SVM_ATOMICS";
                            cap &= ~CL_DEVICE_SVM_ATOMICS;
                        }
                        else
                        {
                            ss << StringUtils::ToString(cap);
                            cap = 0;
                        }

                        if (cap != 0)
                        {
                            ss << '|';
                        }
                    }

                    break;
                }


                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetSizeString(const size_t* size, size_t sizeVal)
{
    if (size == NULL)
    {
        return "NULL";
    }
    else
    {
        std::ostringstream ss;
        ss << '[' << sizeVal << ']';
        return ss.str();
    }
}

std::string CLStringUtils::GetIntString(const cl_uint* intPtr, cl_uint intVal)
{
    if (intPtr == NULL)
    {
        return "NULL";
    }
    else
    {
        std::ostringstream ss;
        ss << '[' << intVal << ']';
        return ss.str();
    }
}

std::string CLStringUtils::GetPartitionPropertyString(const cl_device_partition_property cprop)
{
    switch (cprop)
    {
            CASE(CL_DEVICE_PARTITION_EQUALLY);
            CASE(CL_DEVICE_PARTITION_BY_COUNTS);
            CASE(CL_DEVICE_PARTITION_BY_COUNTS_LIST_END);
            CASE(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN);

        default: return StringUtils::ToString(cprop);
    }
}

std::string CLStringUtils::GetPartitionAffinityDomainString(const cl_device_affinity_domain affinity_domain)
{
    if (affinity_domain == 0)
    {
        return "0";
    }

    std::ostringstream ss;

    cl_device_affinity_domain domain = affinity_domain;

    while (domain)
    {
        if (domain & CL_DEVICE_AFFINITY_DOMAIN_NUMA)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_NUMA";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_NUMA;
        }
        else if (domain & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE;
        }
        else if (domain & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE;
        }
        else if (domain & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE;
        }
        else if (domain & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE;
        }
        else if (domain & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE)
        {
            ss << "CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE";
            domain &= ~CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE;
        }
        else
        {
            ss << StringUtils::ToString(domain);
            domain = 0;
        }

        if (domain != 0)
        {
            ss << '|';
        }
    }

    return ss.str();
}

std::string CLStringUtils::GetPartitionPropertiesString(const std::vector<cl_device_partition_property>& properties, bool include_brackets)
{
    if (properties.size() > 0)
    {
        std::ostringstream ss;

        if (include_brackets)
        {
            ss << '[';
        }

        ss << '{';

        std::vector<cl_device_partition_property>::const_iterator it = properties.begin();
        ss << GetPartitionPropertyString(*it);

        switch (*it++)
        {
            case CL_DEVICE_PARTITION_EQUALLY:
            {
                while (it != properties.end() && *it != 0)
                {
                    ss << ',' << (int)*it;
                    it++;
                }

                break;
            }

            case CL_DEVICE_PARTITION_BY_COUNTS:
            {
                while (it != properties.end() && *it != CL_DEVICE_PARTITION_BY_COUNTS_LIST_END)
                {
                    ss << ',' << *it;
                    it++;
                }

                ss << ",CL_PARTITION_BY_COUNTS_LIST_END";
                break;
            }

            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
            {
                while (it != properties.end() && *it != 0)
                {
                    ss << ',' << GetPartitionAffinityDomainString((cl_device_affinity_domain)*it);
                    it++;
                }

                break;
            }

            default:
            {
                while (it != properties.end() && *it != 0)
                {
                    ss << ',' << StringUtils::ToString(*it);
                    it++;
                }

                break;
            }
        }

        ss << ",0}";

        if (include_brackets)
        {
            ss << ']';
        }

        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetPartitionPropertyExtString(const cl_device_partition_property_ext cprop)
{
    switch (cprop)
    {
            CASE(CL_DEVICE_PARTITION_EQUALLY_EXT);
            CASE(CL_DEVICE_PARTITION_BY_COUNTS_EXT);
            CASE(CL_DEVICE_PARTITION_BY_NAMES_EXT);
            CASE(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT);

        default: return StringUtils::ToString(cprop);
    }
}

std::string CLStringUtils::GetPartitionAffinityDomainExtString(const cl_device_partition_property_ext cprop)
{
    switch (cprop)
    {
            CASE(CL_AFFINITY_DOMAIN_L1_CACHE_EXT);
            CASE(CL_AFFINITY_DOMAIN_L2_CACHE_EXT);
            CASE(CL_AFFINITY_DOMAIN_L3_CACHE_EXT);
            CASE(CL_AFFINITY_DOMAIN_L4_CACHE_EXT);
            CASE(CL_AFFINITY_DOMAIN_NUMA_EXT);
            CASE(CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT);

        default: return StringUtils::ToString(cprop);
    }
}

std::string CLStringUtils::GetPartitionPropertiesExtString(const std::vector<cl_device_partition_property_ext>& properties, bool include_brackets)
{
    if (properties.size() > 0)
    {
        std::ostringstream ss;

        if (include_brackets)
        {
            ss << '[';
        }

        ss << '{';

        std::vector<cl_device_partition_property_ext>::const_iterator it = properties.begin();
        ss << GetPartitionPropertyExtString(*it);

        switch (*it++)
        {
            case CL_DEVICE_PARTITION_EQUALLY_EXT:
            {
                while (it != properties.end() && *it != CL_PROPERTIES_LIST_END_EXT)
                {
                    ss << ',' << *it;
                    it++;
                }

                break;
            }

            case CL_DEVICE_PARTITION_BY_COUNTS_EXT:
            {
                while (it != properties.end() && *it != CL_PARTITION_BY_COUNTS_LIST_END_EXT)
                {
                    ss << ',' << *it;
                    it++;
                }

                ss << ",CL_PARTITION_BY_COUNTS_LIST_END_EXT";
                break;
            }

            case CL_DEVICE_PARTITION_BY_NAMES_EXT:
            {
                while (it != properties.end() && *it != CL_PARTITION_BY_NAMES_LIST_END_EXT)
                {
                    ss << ',' << *it;
                    it++;
                }

                ss << ",CL_PARTITION_BY_NAMES_LIST_END_EXT";
                break;
            }

            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT:
            {
                while (it != properties.end() && *it != CL_PROPERTIES_LIST_END_EXT)
                {
                    ss << ',' << GetPartitionAffinityDomainExtString(*it);
                    it++;
                }

                break;
            }

            default:
            {
                while (it != properties.end() && *it != CL_PROPERTIES_LIST_END_EXT)
                {
                    ss << ',' << StringUtils::ToString(*it);
                    it++;
                }

                break;
            }
        }

        ss << ",CL_PROPERTIES_LIST_END_EXT}";

        if (include_brackets)
        {
            ss << ']';
        }

        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetGLTextureTargetString(const cl_GLenum texture_target)
{
    switch (texture_target)
    {
            CASE(GL_TEXTURE_1D);
            CASE(GL_TEXTURE_1D_ARRAY);
            //         CASE(GL_TEXTURE_BUFFER);
            CASE(GL_TEXTURE_2D);
            CASE(GL_TEXTURE_2D_ARRAY);
            CASE(GL_TEXTURE_3D);
            CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
            CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
            CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
            CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
            CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
            CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
            //         CASE(GL_TEXTURE_RECTANGLE);
            CASE(GL_TEXTURE_RECTANGLE_ARB);

        default: return StringUtils::ToString(texture_target);
    }
}

std::string CLStringUtils::GetGLTextureInfoString(const cl_gl_texture_info param_name)
{
    switch (param_name)
    {
            CASE(CL_GL_TEXTURE_TARGET);
            CASE(CL_GL_MIPMAP_LEVEL);
            CASE(CL_GL_NUM_SAMPLES);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetGLTextureInfoValueString(const cl_gl_texture_info param_name, const void* param_value, cl_int ret_val)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_val == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_GL_TEXTURE_TARGET:
                    ss << GetGLTextureTargetString(*((cl_GLenum*)param_value));
                    break;

                case CL_GL_MIPMAP_LEVEL:
                case CL_GL_NUM_SAMPLES:
                    ss << *((cl_GLint*)param_value);
                    break;

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetGLContextInfoString(const cl_gl_context_info param_name)
{
    switch (param_name)
    {
            CASE(CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR);
            CASE(CL_DEVICES_FOR_GL_CONTEXT_KHR);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetGLContextInfoValueString(const cl_gl_context_info param_name, const size_t param_value_size, const void* param_value, cl_int ret_value)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        if (ret_value == CL_SUCCESS)
        {
            switch (param_name)
            {
                case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
                {
                    ss << StringUtils::ToHexString(*((cl_device_id*)param_value));
                    break;
                }

                case CL_DEVICES_FOR_GL_CONTEXT_KHR:
                {
                    cl_device_id* dids = (cl_device_id*)param_value;
                    size_t num_devices = param_value_size / sizeof(cl_device_id);

                    for (size_t i = 0; i < num_devices; i++)
                    {
                        ss << StringUtils::ToHexString(dids[i]);

                        if (i != num_devices - 1)
                        {
                            ss << ",";
                        }
                    }

                    break;
                }

                default:
                    ss << StringUtils::ToString(*((int*)param_value));
                    break;
            }
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetGLObjectTypeString(cl_gl_object_type* gl_object_type, cl_gl_object_type gl_object_typeVal)
{
    if (gl_object_type == NULL)
    {
        return "NULL";
    }
    else
    {
        std::ostringstream ss;
        ss << '[';

        switch (gl_object_typeVal)
        {
                CASESTR(CL_GL_OBJECT_BUFFER)
                CASESTR(CL_GL_OBJECT_TEXTURE2D)
                CASESTR(CL_GL_OBJECT_TEXTURE3D)
                CASESTR(CL_GL_OBJECT_RENDERBUFFER)

            default:
                ss << StringUtils::ToString(gl_object_typeVal);
                break;
        }

        ss << ']';
        return ss.str();
    }
}

#ifdef _WIN32
std::string CLStringUtils::GetD3D10DeviceSourceString(cl_d3d10_device_source_khr d3d_device_source)
{
    switch (d3d_device_source)
    {
            CASE(CL_D3D10_DEVICE_KHR);
            CASE(CL_D3D10_DXGI_ADAPTER_KHR);

        default: return StringUtils::ToString(d3d_device_source);
    }
}

std::string CLStringUtils::GetD3D10DeviceSetString(cl_d3d10_device_set_khr d3d_device_set)
{
    switch (d3d_device_set)
    {
            CASE(CL_PREFERRED_DEVICES_FOR_D3D10_KHR);
            CASE(CL_ALL_DEVICES_FOR_D3D10_KHR);

        default: return StringUtils::ToString(d3d_device_set);
    }
}
#endif

std::string CLStringUtils::GetBuildOptionsString(const std::string& strOptions, const char* options, const std::string& strOverriddenOptions, bool bOptionsAppended, bool bIsLink)
{
    std::string strOrigVal = GetQuotedString(strOptions, options);

    if (strOverriddenOptions.empty())
    {
        return strOrigVal;
    }

    std::ostringstream ss;

    // show the actual options used
    ss << "\"" << strOverriddenOptions << "\"";

    // and show a comment with the original vlaue and an explanation of why the actual options are different than the original options
    ss << " " << "/* original value of " << strOrigVal << " modified because environment variable \"AMD_OCL_";

    if (bIsLink)
    {
        ss << "LINK";
    }
    else
    {
        ss << "BUILD";
    }

    ss << "_OPTIONS";

    if (bOptionsAppended)
    {
        ss << "_APPEND";
    }

    ss << "\" was set */";

    return ss.str();
}

std::string CLStringUtils::GetImageDescString(cl_image_desc* imageDescriptor)
{

    if (imageDescriptor == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;
    ss << "[{"
       << GetMemObjectTypeString(imageDescriptor->image_type) << ","
       << imageDescriptor->image_width << ","
       << imageDescriptor->image_height << ","
       << imageDescriptor->image_depth << ","
       << imageDescriptor->image_array_size << ","
       << imageDescriptor->image_row_pitch << ","
       << imageDescriptor->image_slice_pitch << ","
       << imageDescriptor->num_mip_levels << ","
       << imageDescriptor->num_samples << ","
       << StringUtils::ToHexString(imageDescriptor->buffer)
       << "}]";
    return ss.str();
}
std::string CLStringUtils::GetDeviceTopologyAMDTypeString(const cl_uint device_topology_type)
{
    switch (device_topology_type)
    {
            CASE(CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD);

        default: return StringUtils::ToString(device_topology_type);
    }
}

std::string CLStringUtils::GetDeviceTopologyAMDString(cl_device_topology_amd* device_topology_amd, bool include_brackets)
{
    if (device_topology_amd == NULL)
    {
        return "NULL";
    }

    std::ostringstream ss;

    if (include_brackets)
    {
        ss << '[';
    }

    ss << "{"
       << GetDeviceTopologyAMDTypeString(device_topology_amd->raw.type) << ",";

    if (device_topology_amd->raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD)
    {
        ss << "{";

        for (int i = 0; i < 17; i++)
        {
            ss << StringUtils::ToString((cl_int)device_topology_amd->pcie.unused[i]);

            if (i < 16)
            {
                ss << ",";
            }
        }

        ss << "}" << ","
           << StringUtils::ToString((cl_int)device_topology_amd->pcie.bus) << ","
           << StringUtils::ToString((cl_int)device_topology_amd->pcie.device) << ","
           << StringUtils::ToString((cl_int)device_topology_amd->pcie.function);
    }
    else
    {
        ss << "{";

        for (int i = 0; i < 5; i++)
        {
            ss << StringUtils::ToString(device_topology_amd->raw.data[i]);

            if (i < 4)
            {
                ss << ",";
            }
        }

        ss << "}";
    }

    ss << "}";

    if (include_brackets)
    {
        ss << ']';
    }

    return ss.str();
}

std::string CLStringUtils::GetKernelExecInfoString(const cl_kernel_exec_info param_name)
{
    switch (param_name)
    {
            CASE(CL_KERNEL_EXEC_INFO_SVM_PTRS);
            CASE(CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM);

        default: return StringUtils::ToString(param_name);
    }
}

std::string CLStringUtils::GetKernelExecInfoValueString(const cl_kernel_exec_info param_name, const void* param_value, cl_int ret_val, size_t param_value_size)
{
    if (param_value != NULL)
    {
        std::ostringstream ss;
        ss << '[';

        switch (param_name)
        {
            case CL_KERNEL_EXEC_INFO_SVM_PTRS:
            {
                if (ret_val == CL_SUCCESS)
                {
                    size_t num_ptrs = param_value_size / sizeof(void*);
                    intptr_t* val = (intptr_t*)param_value;

                    for (size_t i = 0; i < num_ptrs; i++)
                    {
                        ss << StringUtils::ToHexString((void*)(*val));

                        if (i < num_ptrs - 1)
                        {
                            ss << ",";
                        }

                        val++;
                    }
                }
                else
                {
                    ss << StringUtils::ToHexString(param_value);
                }

                break;
            }

            case CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM:
                ss << GetBoolString(*(cl_bool*)param_value);
                break;

            default:
                ss << StringUtils::ToString(*((int*)param_value));
                break;
        }

        ss << ']';
        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

std::string CLStringUtils::GetPointerListString(void** pointers, const std::vector<void*>& vecPointers, bool include_brackets)
{
    if (pointers != NULL)
    {
        std::ostringstream ss;

        if (include_brackets)
        {
            ss << '[';
        }

        for (size_t i = 0; i != vecPointers.size(); i++)
        {
            ss << StringUtils::ToHexString(vecPointers[i]);

            if (i != vecPointers.size() - 1)
            {
                ss << ',';
            }
        }

        if (include_brackets)
        {
            ss << ']';
        }

        return ss.str();
    }
    else
    {
        return "NULL";
    }
}

