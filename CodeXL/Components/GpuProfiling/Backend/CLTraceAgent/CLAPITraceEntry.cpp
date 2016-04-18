//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This contains the initialization function for API tracing.
//==============================================================================

#include <CL/opencl.h>
#include <vector>
#include "CLAPITraceEntry.h"
#include "CLAPIInfoManager.h"
#include "CLTraceAgent.h"
#include "PMCSamplerManager.h"
#include "../Common/GlobalSettings.h"
#include "../Common/OSUtils.h"
#include "../Common/Defs.h"
#include "../Common/Logger.h"
#include "../CLCommon/CLUtils.h"

using namespace std;
using namespace GPULogger;

bool g_bQueryRetStat = false;
#define REPLACE_IF_NULL(p) cl_int newRetVal; if(!p && g_bQueryRetStat) {p = &newRetVal;}

/// Assigns a real function pointer to an entry in g_realExtensionFunctionTable and
/// returns the CL_API_TRACE_* version
/// \param pFuncName the name of the extension function whose pointer should be assigned
/// \param pRealFuncPtr the address of the real function pointer for the specified extension
/// \return the address of the CL_API_TRACE_ version for the specified extension function
void* AssignExtensionFunctionPointer(const char* pFuncName, void* pRealFuncPtr);

void SetGlobalTraceFlags(bool queryRetStat, bool collapseClGetEventInfo)
{
    g_bQueryRetStat = queryRetStat;
    CLAPI_clGetEventInfo::ms_collapseCalls = collapseClGetEventInfo;
}

cl_int CL_API_CALL CL_API_TRACE_clGetPlatformIDs(
    cl_uint           num_entries ,
    cl_platform_id*   platform_list ,
    cl_uint*          num_platforms)
{
    bool replaced_null_param = num_platforms == NULL;

    // provide a non-null var for num_platforms, only if platform_list is non-null
    // or num_entries is non-zero. If platform_list is null and num_entries is zero,
    // then leave num_platforms as null, so that the real API can return CL_INVALID_VALUE
    cl_uint substituted_ret;

    if (replaced_null_param && (platform_list != NULL || num_entries != 0))
    {
        num_platforms = &substituted_ret;
    }

    CLAPI_clGetPlatformIDs* pAPIInfo = new(nothrow) CLAPI_clGetPlatformIDs();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetPlatformIDs(
                     num_entries,
                     platform_list,
                     num_platforms);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     num_entries,
                     platform_list,
                     num_platforms,
                     replaced_null_param,
                     ret);
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetPlatformInfo(
    cl_platform_id    platform ,
    cl_platform_info  param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetPlatformInfo* pAPIInfo = new(nothrow) CLAPI_clGetPlatformInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetPlatformInfo(
                     platform,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     platform,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetDeviceIDs(
    cl_platform_id    platform ,
    cl_device_type    device_type ,
    cl_uint           num_entries ,
    cl_device_id*     device_list ,
    cl_uint*          num_devices)
{
    bool replaced_null_param = num_devices == NULL;

    // provide a non-null var for num_devices, only if device_list is non-null
    // or num_entries is non-zero. If device_list is null and num_entries is zero,
    // then leave num_devices as null, so that the real API can return CL_INVALID_VALUE
    cl_uint substituted_ret;

    if (replaced_null_param && (device_list != NULL || num_entries != 0))
    {
        num_devices = &substituted_ret;
    }

    CLAPI_clGetDeviceIDs* pAPIInfo = new(nothrow) CLAPI_clGetDeviceIDs();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetDeviceIDs(
                     platform,
                     device_type,
                     num_entries,
                     device_list,
                     num_devices);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     platform,
                     device_type,
                     num_entries,
                     device_list,
                     num_devices,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetDeviceInfo(
    cl_device_id     device ,
    cl_device_info   param_name ,
    size_t           param_value_size ,
    void*            param_value ,
    size_t*          param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetDeviceInfo* pAPIInfo = new(nothrow) CLAPI_clGetDeviceInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetDeviceInfo(
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_context CL_API_CALL CL_API_TRACE_clCreateContext(
    const cl_context_properties*   properties ,
    cl_uint                        num_devices ,
    const cl_device_id*            device_list ,
    void (CL_CALLBACK*   pfn_notify)(const char*, const void*, size_t, void*) ,
    void*                          user_data ,
    cl_int*                        errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateContext* pAPIInfo = new(nothrow) CLAPI_clCreateContext();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_context ret = g_nextDispatchTable.CreateContext(
                         properties,
                         num_devices,
                         device_list,
                         pfn_notify,
                         user_data,
                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     properties,
                     num_devices,
                     device_list,
                     pfn_notify,
                     user_data,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_context CL_API_CALL CL_API_TRACE_clCreateContextFromType(
    const cl_context_properties*   properties ,
    cl_device_type                 device_type ,
    void (CL_CALLBACK*       pfn_notify)(const char*, const void*, size_t, void*) ,
    void*                          user_data ,
    cl_int*                        errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateContextFromType* pAPIInfo = new(nothrow) CLAPI_clCreateContextFromType();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_context ret = g_nextDispatchTable.CreateContextFromType(
                         properties,
                         device_type,
                         pfn_notify,
                         user_data,
                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     properties,
                     device_type,
                     pfn_notify,
                     user_data,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainContext(cl_context  context)
{
    CLAPI_clRetainContext* pAPIInfo = new(nothrow) CLAPI_clRetainContext();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainContext(context);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseContext(cl_context  context)
{
    CLAPI_clReleaseContext* pAPIInfo = new(nothrow) CLAPI_clReleaseContext();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseContext(context);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);

    //Flush here since dll unload doesn't work
    if (CLAPIInfoManager::Instance()->IsTimeOutMode())
    {
        CLAPIInfoManager::Instance()->StopTimer();
        CLAPIInfoManager::Instance()->TrySwapBuffer();
        CLAPIInfoManager::Instance()->FlushTraceData(true);
        CLAPIInfoManager::Instance()->TrySwapBuffer();
        CLAPIInfoManager::Instance()->FlushTraceData(true);
#ifdef NON_BLOCKING_TIMEOUT
        CLEventManager::Instance()->TrySwapBuffer();
        CLEventManager::Instance()->FlushTraceData();
#endif
        CLAPIInfoManager::Instance()->ResumeTimer();
    }

    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetContextInfo(
    cl_context          context ,
    cl_context_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetContextInfo* pAPIInfo = new(nothrow) CLAPI_clGetContextInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetContextInfo(
                     context,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_command_queue CL_API_CALL CL_API_TRACE_clCreateCommandQueue(
    cl_context                      context ,
    cl_device_id                    device ,
    cl_command_queue_properties     properties ,
    cl_int*                         errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateCommandQueue* pAPIInfo = new(nothrow) CLAPI_clCreateCommandQueue();

    // We need to set CL_QUEUE_PROFILING_ENABLE in order to get correct GPU timestamp for
    // clEnqueueMapBuffer/clEnqueueUnmapMemObject
    cl_command_queue_properties newProps = properties;
    newProps |= CL_QUEUE_PROFILING_ENABLE;

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_command_queue ret = g_nextDispatchTable.CreateCommandQueue(
                               context,
                               device,
                               newProps,
                               errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     device,
                     properties,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainCommandQueue(cl_command_queue  command_queue)
{
    CLAPI_clRetainCommandQueue* pAPIInfo = new(nothrow) CLAPI_clRetainCommandQueue();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainCommandQueue(command_queue);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseCommandQueue(cl_command_queue  command_queue)
{
    CLAPI_clReleaseCommandQueue* pAPIInfo = new(nothrow) CLAPI_clReleaseCommandQueue();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseCommandQueue(command_queue);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetCommandQueueInfo(
    cl_command_queue       command_queue ,
    cl_command_queue_info  param_name ,
    size_t                 param_value_size ,
    void*                  param_value ,
    size_t*                param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetCommandQueueInfo* pAPIInfo = new(nothrow) CLAPI_clGetCommandQueueInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetCommandQueueInfo(
                     command_queue,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    if (param_value != NULL && param_name == CL_QUEUE_PROPERTIES && ret == CL_SUCCESS)
    {
        // Remove CL_QUEUE_PROFILING_ENABLE if user didn't set it when creating command queue.
        const CLAPI_clCreateCommandQueueBase* pQueue = CLAPIInfoManager::Instance()->GetCreateCommandQueueAPIObj(command_queue);

        if (pQueue != NULL)
        {
            if (!pQueue->UserSetProfileFlag())
            {
                // unset it
                cl_command_queue_properties* retProps = static_cast<cl_command_queue_properties*>(param_value);
                (*retProps) ^= CL_QUEUE_PROFILING_ENABLE;
            }
        }
    }

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetCommandQueueProperty(
    cl_command_queue             command_queue,
    cl_command_queue_properties  properties,
    cl_bool                      enable,
    cl_command_queue_properties* old_properties)
{
    cl_command_queue_properties newprops = properties;

    if ((properties & CL_QUEUE_PROFILING_ENABLE) && (enable == CL_FALSE))
    {
        // always enable CL_QUEUE_PROFILING_ENABLE
        newprops ^= CL_QUEUE_PROFILING_ENABLE;
    }

    CLAPI_clSetCommandQueueProperty* pAPIInfo = new(nothrow) CLAPI_clSetCommandQueueProperty();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetCommandQueueProperty(command_queue,
                                                             newprops,
                                                             enable,
                                                             old_properties);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    if (old_properties != NULL && ret == CL_SUCCESS)
    {
        // if old props is queried, if user never set profiling flag, unset it
        const CLAPI_clCreateCommandQueueBase* pQueue = CLAPIInfoManager::Instance()->GetCreateCommandQueueAPIObj(command_queue);

        if (pQueue != NULL)
        {
            if (!pQueue->UserSetProfileFlag())
            {
                (*old_properties) ^= CL_QUEUE_PROFILING_ENABLE;
            }
        }
    }

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     properties,
                     enable,
                     old_properties,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateBuffer(
    cl_context    context ,
    cl_mem_flags  flags ,
    size_t        size ,
    void*         host_ptr ,
    cl_int*       errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateBuffer* pAPIInfo = new(nothrow) CLAPI_clCreateBuffer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateBuffer(
                     context,
                     flags,
                     size,
                     host_ptr,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     size,
                     host_ptr,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateSubBuffer(
    cl_mem                    buffer ,
    cl_mem_flags              flags ,
    cl_buffer_create_type     buffer_create_type ,
    const void*               buffer_create_info ,
    cl_int*                   errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateSubBuffer* pAPIInfo = new(nothrow) CLAPI_clCreateSubBuffer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateSubBuffer(
                     buffer,
                     flags,
                     buffer_create_type,
                     buffer_create_info,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     buffer,
                     flags,
                     buffer_create_type,
                     buffer_create_info,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateImage2D(
    cl_context               context ,
    cl_mem_flags             flags ,
    const cl_image_format*   image_format ,
    size_t                   image_width ,
    size_t                   image_height ,
    size_t                   image_row_pitch ,
    void*                    host_ptr ,
    cl_int*                  errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateImage2D* pAPIInfo = new(nothrow) CLAPI_clCreateImage2D();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateImage2D(
                     context,
                     flags,
                     image_format,
                     image_width,
                     image_height,
                     image_row_pitch,
                     host_ptr,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     image_format,
                     image_width,
                     image_height,
                     image_row_pitch,
                     host_ptr,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateImage3D(
    cl_context               context ,
    cl_mem_flags             flags ,
    const cl_image_format*   image_format ,
    size_t                   image_width ,
    size_t                   image_height ,
    size_t                   image_depth ,
    size_t                   image_row_pitch ,
    size_t                   image_slice_pitch ,
    void*                    host_ptr ,
    cl_int*                  errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateImage3D* pAPIInfo = new(nothrow) CLAPI_clCreateImage3D();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateImage3D(
                     context,
                     flags,
                     image_format,
                     image_width,
                     image_height,
                     image_depth,
                     image_row_pitch,
                     image_slice_pitch,
                     host_ptr,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     image_format,
                     image_width,
                     image_height,
                     image_depth,
                     image_row_pitch,
                     image_slice_pitch,
                     host_ptr,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainMemObject(cl_mem  memobj)
{
    CLAPI_clRetainMemObject* pAPIInfo = new(nothrow) CLAPI_clRetainMemObject();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainMemObject(memobj);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseMemObject(cl_mem memobj)
{
    CLAPI_clReleaseMemObject* pAPIInfo = new(nothrow) CLAPI_clReleaseMemObject();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseMemObject(memobj);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetSupportedImageFormats(
    cl_context            context ,
    cl_mem_flags          flags ,
    cl_mem_object_type    image_type ,
    cl_uint               num_entries ,
    cl_image_format*      image_formats ,
    cl_uint*              num_image_formats)
{
    bool replaced_null_param = num_image_formats == NULL;

    cl_uint substituted_ret;

    if (replaced_null_param)
    {
        num_image_formats = &substituted_ret;
    }

    CLAPI_clGetSupportedImageFormats* pAPIInfo = new(nothrow) CLAPI_clGetSupportedImageFormats();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetSupportedImageFormats(
                     context,
                     flags,
                     image_type,
                     num_entries,
                     image_formats,
                     num_image_formats);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     image_type,
                     num_entries,
                     image_formats,
                     num_image_formats,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetMemObjectInfo(
    cl_mem            memobj ,
    cl_mem_info       param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetMemObjectInfo* pAPIInfo = new(nothrow) CLAPI_clGetMemObjectInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetMemObjectInfo(
                     memobj,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetImageInfo(
    cl_mem            image ,
    cl_image_info     param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetImageInfo* pAPIInfo = new(nothrow) CLAPI_clGetImageInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetImageInfo(
                     image,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     image,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetMemObjectDestructorCallback(
    cl_mem  memobj ,
    void (CL_CALLBACK* pfn_notify)(cl_mem  memobj , void* user_data) ,
    void* user_data)
{
    CLAPI_clSetMemObjectDestructorCallback* pAPIInfo = new(nothrow) CLAPI_clSetMemObjectDestructorCallback();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetMemObjectDestructorCallback(
                     memobj,
                     pfn_notify,
                     user_data);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     pfn_notify,
                     user_data,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_sampler CL_API_CALL CL_API_TRACE_clCreateSampler(
    cl_context           context ,
    cl_bool              normalized_coords ,
    cl_addressing_mode   addressing_mode ,
    cl_filter_mode       filter_mode ,
    cl_int*              errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateSampler* pAPIInfo = new(nothrow) CLAPI_clCreateSampler();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_sampler ret = g_nextDispatchTable.CreateSampler(
                         context,
                         normalized_coords,
                         addressing_mode,
                         filter_mode,
                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     normalized_coords,
                     addressing_mode,
                     filter_mode,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainSampler(cl_sampler  sampler)
{
    CLAPI_clRetainSampler* pAPIInfo = new(nothrow) CLAPI_clRetainSampler();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainSampler(sampler);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     sampler,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseSampler(cl_sampler  sampler)
{
    CLAPI_clReleaseSampler* pAPIInfo = new(nothrow) CLAPI_clReleaseSampler();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseSampler(sampler);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     sampler,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetSamplerInfo(
    cl_sampler          sampler ,
    cl_sampler_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetSamplerInfo* pAPIInfo = new(nothrow) CLAPI_clGetSamplerInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetSamplerInfo(
                     sampler,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     sampler,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_program CL_API_CALL CL_API_TRACE_clCreateProgramWithSource(
    cl_context         context ,
    cl_uint            count ,
    const char**       strings ,
    const size_t*      lengths ,
    cl_int*            errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateProgramWithSource* pAPIInfo = new(nothrow) CLAPI_clCreateProgramWithSource();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_program ret = g_nextDispatchTable.CreateProgramWithSource(
                         context,
                         count,
                         strings,
                         lengths,
                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     count,
                     strings,
                     lengths,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_program CL_API_CALL CL_API_TRACE_clCreateProgramWithBinary(
    cl_context                      context ,
    cl_uint                         num_devices ,
    const cl_device_id*             device_list ,
    const size_t*                   lengths ,
    const unsigned char**           binaries ,
    cl_int*                         binary_status ,
    cl_int*                         errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateProgramWithBinary* pAPIInfo = new(nothrow) CLAPI_clCreateProgramWithBinary();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_program ret = g_nextDispatchTable.CreateProgramWithBinary(
                         context,
                         num_devices,
                         device_list,
                         lengths,
                         binaries,
                         binary_status,
                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     num_devices,
                     device_list,
                     lengths,
                     binaries,
                     binary_status,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainProgram(cl_program  program)
{
    CLAPI_clRetainProgram* pAPIInfo = new(nothrow) CLAPI_clRetainProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainProgram(program);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseProgram(cl_program  program)
{
    CLAPI_clReleaseProgram* pAPIInfo = new(nothrow) CLAPI_clReleaseProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseProgram(program);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clBuildProgram(
    cl_program            program ,
    cl_uint               num_devices ,
    const cl_device_id*   device_list ,
    const char*           options ,
    void (CL_CALLBACK*    pfn_notify)(cl_program  program , void*   user_data) ,
    void*                 user_data)
{
    CLAPI_clBuildProgram* pAPIInfo = new(nothrow) CLAPI_clBuildProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.BuildProgram(
                     program,
                     num_devices,
                     device_list,
                     options,
                     pfn_notify,
                     user_data);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     num_devices,
                     device_list,
                     options,
                     pfn_notify,
                     user_data,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clUnloadCompiler(void)
{
    CLAPI_clUnloadCompiler* pAPIInfo = new(nothrow) CLAPI_clUnloadCompiler();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.UnloadCompiler();

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetProgramInfo(
    cl_program          program ,
    cl_program_info     param_name ,
    size_t              param_value_size ,
    void*               param_value ,
    size_t*             param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetProgramInfo* pAPIInfo = new(nothrow) CLAPI_clGetProgramInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetProgramInfo(
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetProgramBuildInfo(
    cl_program             program ,
    cl_device_id           device ,
    cl_program_build_info  param_name ,
    size_t                 param_value_size ,
    void*                  param_value ,
    size_t*                param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetProgramBuildInfo* pAPIInfo = new(nothrow) CLAPI_clGetProgramBuildInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetProgramBuildInfo(
                     program,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_kernel CL_API_CALL CL_API_TRACE_clCreateKernel(
    cl_program       program ,
    const char*      kernel_name ,
    cl_int*          errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateKernel* pAPIInfo = new(nothrow) CLAPI_clCreateKernel();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_kernel ret = g_nextDispatchTable.CreateKernel(
                        program,
                        kernel_name,
                        errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     kernel_name,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clCreateKernelsInProgram(
    cl_program      program ,
    cl_uint         num_kernels ,
    cl_kernel*      kernels ,
    cl_uint*        num_kernels_ret)
{
    bool replaced_null_param = num_kernels_ret == NULL;

    cl_uint substituted_ret;

    if (replaced_null_param)
    {
        num_kernels_ret = &substituted_ret;
    }

    CLAPI_clCreateKernelsInProgram* pAPIInfo = new(nothrow) CLAPI_clCreateKernelsInProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.CreateKernelsInProgram(
                     program,
                     num_kernels,
                     kernels,
                     num_kernels_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     num_kernels,
                     kernels,
                     num_kernels_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainKernel(cl_kernel     kernel)
{
    CLAPI_clRetainKernel* pAPIInfo = new(nothrow) CLAPI_clRetainKernel();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainKernel(kernel);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseKernel(cl_kernel    kernel)
{
    CLAPI_clReleaseKernel* pAPIInfo = new(nothrow) CLAPI_clReleaseKernel();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseKernel(kernel);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetKernelArg(
    cl_kernel     kernel ,
    cl_uint       arg_index ,
    size_t        arg_size ,
    const void*   arg_value)
{
    CLAPI_clSetKernelArg* pAPIInfo = new(nothrow) CLAPI_clSetKernelArg();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetKernelArg(
                     kernel,
                     arg_index,
                     arg_size,
                     arg_value);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     arg_index,
                     arg_size,
                     arg_value,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetKernelInfo(
    cl_kernel        kernel ,
    cl_kernel_info   param_name ,
    size_t           param_value_size ,
    void*            param_value ,
    size_t*          param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetKernelInfo* pAPIInfo = new(nothrow) CLAPI_clGetKernelInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetKernelInfo(
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetKernelWorkGroupInfo(
    cl_kernel                   kernel ,
    cl_device_id                device ,
    cl_kernel_work_group_info   param_name ,
    size_t                      param_value_size ,
    void*                       param_value ,
    size_t*                     param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetKernelWorkGroupInfo* pAPIInfo = new(nothrow) CLAPI_clGetKernelWorkGroupInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetKernelWorkGroupInfo(
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     device,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clWaitForEvents(
    cl_uint              num_events ,
    const cl_event*      event_list)
{
    CLAPI_clWaitForEvents* pAPIInfo = new(nothrow) CLAPI_clWaitForEvents();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.WaitForEvents(
                     num_events,
                     event_list);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     num_events,
                     event_list,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetEventInfo(
    cl_event          event ,
    cl_event_info     param_name ,
    size_t            param_value_size ,
    void*             param_value ,
    size_t*           param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetEventInfo* pAPIInfo = new(nothrow) CLAPI_clGetEventInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetEventInfo(
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    /*
       // allow users to tell the profiler to not trace clGetEventInfo apis by setting an env var
       // only skip calls made with CL_EVENT_COMMAND_EXECUTION_STATUS that report a status other than CL_COMPLETE
       static bool skipCLGetEventInfo = !OSUtils::Instance()->GetEnvVar("NOTRACE_CLGETEVENTINFO").empty();
       if (skipCLGetEventInfo && ret == CL_SUCCESS && param_name == CL_EVENT_COMMAND_EXECUTION_STATUS && param_value != NULL && *(cl_int*)param_value != CL_COMPLETE)
       {
          return ret;
       }
    */

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_event CL_API_CALL CL_API_TRACE_clCreateUserEvent(
    cl_context     context ,
    cl_int*        errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateUserEvent* pAPIInfo = new(nothrow) CLAPI_clCreateUserEvent();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_event ret = g_nextDispatchTable.CreateUserEvent(
                       context,
                       errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainEvent(cl_event  event)
{
    CLAPI_clRetainEvent* pAPIInfo = new(nothrow) CLAPI_clRetainEvent();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainEvent(event);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseEvent(cl_event  event)
{

#ifdef _DEBUG_REF_COUNT_
    cl_uint refC;
    GetRealDispatchTable()->GetEventInfo(event, CL_EVENT_REFERENCE_COUNT, sizeof(cl_uint), &refC, NULL);
#endif

    CLAPI_clReleaseEvent* pAPIInfo = new(nothrow) CLAPI_clReleaseEvent();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseEvent(event);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     ret);

#ifdef _DEBUG_REF_COUNT_
    pAPIInfo->m_uiRefCount = refC - 1;
#endif

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetUserEventStatus(
    cl_event    event ,
    cl_int      execution_status)
{
    CLAPI_clSetUserEventStatus* pAPIInfo = new(nothrow) CLAPI_clSetUserEventStatus();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetUserEventStatus(
                     event,
                     execution_status);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     execution_status,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetEventCallback(
    cl_event     event ,
    cl_int       command_exec_callback_type ,
    void (CL_CALLBACK*   pfn_notify)(cl_event, cl_int, void*) ,
    void*        user_data)
{
    CLAPI_clSetEventCallback* pAPIInfo = new(nothrow) CLAPI_clSetEventCallback();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetEventCallback(
                     event,
                     command_exec_callback_type,
                     pfn_notify,
                     user_data);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     command_exec_callback_type,
                     pfn_notify,
                     user_data,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetEventProfilingInfo(
    cl_event             event ,
    cl_profiling_info    param_name ,
    size_t               param_value_size ,
    void*                param_value ,
    size_t*              param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetEventProfilingInfo* pAPIInfo = new(nothrow) CLAPI_clGetEventProfilingInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetEventProfilingInfo(
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     event,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clFlush(cl_command_queue  command_queue)
{
    CLAPI_clFlush* pAPIInfo = new(nothrow) CLAPI_clFlush();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.Flush(command_queue);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clFinish(cl_command_queue  command_queue)
{
    CLAPI_clFinish* pAPIInfo = new(nothrow) CLAPI_clFinish();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.Finish(command_queue);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueReadBuffer(
    cl_command_queue     command_queue ,
    cl_mem               buffer ,
    cl_bool              blocking_read ,
    size_t               offset ,
    size_t               cb ,
    void*                ptr ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{
    CLAPI_clEnqueueReadBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueReadBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueReadBuffer(command_queue,
                                                     buffer,
                                                     blocking_read,
                                                     offset,
                                                     cb,
                                                     ptr,
                                                     num_events_in_wait_list,
                                                     event_wait_list,
                                                     event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     buffer,
                     blocking_read,
                     offset,
                     cb,
                     ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueReadBufferRect(
    cl_command_queue     command_queue ,
    cl_mem               buffer ,
    cl_bool              blocking_read ,
    const size_t         buffer_offset[3] ,
    const size_t         host_offset[3] ,
    const size_t         region[3] ,
    size_t               buffer_row_pitch ,
    size_t               buffer_slice_pitch ,
    size_t               host_row_pitch ,
    size_t               host_slice_pitch ,
    void*                ptr ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{
    CLAPI_clEnqueueReadBufferRect* pAPIInfo = new(nothrow) CLAPI_clEnqueueReadBufferRect();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueReadBufferRect(command_queue,
                                                         buffer,
                                                         blocking_read,
                                                         buffer_offset,
                                                         host_offset,
                                                         region,
                                                         buffer_row_pitch,
                                                         buffer_slice_pitch,
                                                         host_row_pitch,
                                                         host_slice_pitch,
                                                         ptr,
                                                         num_events_in_wait_list,
                                                         event_wait_list,
                                                         event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     buffer,
                     blocking_read,
                     buffer_offset,
                     host_offset,
                     region,
                     buffer_row_pitch,
                     buffer_slice_pitch,
                     host_row_pitch,
                     host_slice_pitch,
                     ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueWriteBuffer(
    cl_command_queue    command_queue ,
    cl_mem              buffer ,
    cl_bool             blocking_write ,
    size_t              offset ,
    size_t              cb ,
    const void*         ptr ,
    cl_uint             num_events_in_wait_list ,
    const cl_event*     event_wait_list ,
    cl_event*           event)
{
    CLAPI_clEnqueueWriteBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueWriteBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueWriteBuffer(command_queue,
                                                      buffer,
                                                      blocking_write,
                                                      offset,
                                                      cb,
                                                      ptr,
                                                      num_events_in_wait_list,
                                                      event_wait_list,
                                                      event);
    }


    cl_uint ret = pAPIInfo->Create(
                      command_queue,
                      buffer,
                      blocking_write,
                      offset,
                      cb,
                      ptr,
                      num_events_in_wait_list,
                      event_wait_list,
                      event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueWriteBufferRect(
    cl_command_queue     command_queue ,
    cl_mem               buffer ,
    cl_bool              blocking_read ,
    const size_t         buffer_offset[3] ,
    const size_t         host_offset[3] ,
    const size_t         region[3] ,
    size_t               buffer_row_pitch ,
    size_t               buffer_slice_pitch ,
    size_t               host_row_pitch ,
    size_t               host_slice_pitch ,
    const void*          ptr ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{
    CLAPI_clEnqueueWriteBufferRect* pAPIInfo = new(nothrow) CLAPI_clEnqueueWriteBufferRect();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueWriteBufferRect(command_queue,
                                                          buffer,
                                                          blocking_read,
                                                          buffer_offset,
                                                          host_offset,
                                                          region,
                                                          buffer_row_pitch,
                                                          buffer_slice_pitch,
                                                          host_row_pitch,
                                                          host_slice_pitch,
                                                          ptr,
                                                          num_events_in_wait_list,
                                                          event_wait_list,
                                                          event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     buffer,
                     blocking_read,
                     buffer_offset,
                     host_offset,
                     region,
                     buffer_row_pitch,
                     buffer_slice_pitch,
                     host_row_pitch,
                     host_slice_pitch,
                     ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueCopyBuffer(
    cl_command_queue     command_queue ,
    cl_mem               src_buffer ,
    cl_mem               dst_buffer ,
    size_t               src_offset ,
    size_t               dst_offset ,
    size_t               cb ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{
    CLAPI_clEnqueueCopyBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueCopyBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueCopyBuffer(command_queue,
                                                     src_buffer,
                                                     dst_buffer,
                                                     src_offset,
                                                     dst_offset,
                                                     cb,
                                                     num_events_in_wait_list,
                                                     event_wait_list,
                                                     event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     src_buffer,
                     dst_buffer,
                     src_offset,
                     dst_offset,
                     cb,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueCopyBufferRect(
    cl_command_queue     command_queue ,
    cl_mem               src_buffer ,
    cl_mem               dst_buffer ,
    const size_t         src_origin[3] ,
    const size_t         dst_origin[3] ,
    const size_t         region[3] ,
    size_t               src_row_pitch ,
    size_t               src_slice_pitch ,
    size_t               dst_row_pitch ,
    size_t               dst_slice_pitch ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{
    CLAPI_clEnqueueCopyBufferRect* pAPIInfo = new(nothrow) CLAPI_clEnqueueCopyBufferRect();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueCopyBufferRect(command_queue,
                                                         src_buffer,
                                                         dst_buffer,
                                                         src_origin,
                                                         dst_origin,
                                                         region,
                                                         src_row_pitch,
                                                         src_slice_pitch,
                                                         dst_row_pitch,
                                                         dst_slice_pitch,
                                                         num_events_in_wait_list,
                                                         event_wait_list,
                                                         event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     src_buffer,
                     dst_buffer,
                     src_origin,
                     dst_origin,
                     region,
                     src_row_pitch,
                     src_slice_pitch,
                     dst_row_pitch,
                     dst_slice_pitch,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueReadImage(
    cl_command_queue      command_queue ,
    cl_mem                image ,
    cl_bool               blocking_read ,
    const size_t          origin[3] ,
    const size_t          region[3] ,
    size_t                row_pitch ,
    size_t                slice_pitch ,
    void*                 ptr ,
    cl_uint               num_events_in_wait_list ,
    const cl_event*       event_wait_list ,
    cl_event*             event)
{


    CLAPI_clEnqueueReadImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueReadImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueReadImage(command_queue,
                                                    image,
                                                    blocking_read,
                                                    origin,
                                                    region,
                                                    row_pitch,
                                                    slice_pitch,
                                                    ptr,
                                                    num_events_in_wait_list,
                                                    event_wait_list,
                                                    event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     image,
                     blocking_read,
                     origin,
                     region,
                     row_pitch,
                     slice_pitch,
                     ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueWriteImage(
    cl_command_queue     command_queue ,
    cl_mem               image ,
    cl_bool              blocking_write ,
    const size_t         origin[3] ,
    const size_t         region[3] ,
    size_t               input_row_pitch ,
    size_t               input_slice_pitch ,
    const void*          ptr ,
    cl_uint              num_events_in_wait_list ,
    const cl_event*      event_wait_list ,
    cl_event*            event)
{

    CLAPI_clEnqueueWriteImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueWriteImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueWriteImage(command_queue,
                                                     image,
                                                     blocking_write,
                                                     origin,
                                                     region,
                                                     input_row_pitch,
                                                     input_slice_pitch,
                                                     ptr,
                                                     num_events_in_wait_list,
                                                     event_wait_list,
                                                     event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     image,
                     blocking_write,
                     origin,
                     region,
                     input_row_pitch,
                     input_slice_pitch,
                     ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueCopyImage(
    cl_command_queue      command_queue ,
    cl_mem                src_image ,
    cl_mem                dst_image ,
    const size_t          src_origin[3] ,
    const size_t          dst_origin[3] ,
    const size_t          region[3] ,
    cl_uint               num_events_in_wait_list ,
    const cl_event*       event_wait_list ,
    cl_event*             event)
{

    CLAPI_clEnqueueCopyImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueCopyImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueCopyImage(command_queue,
                                                    src_image,
                                                    dst_image,
                                                    src_origin,
                                                    dst_origin,
                                                    region,
                                                    num_events_in_wait_list,
                                                    event_wait_list,
                                                    event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     src_image,
                     dst_image,
                     src_origin,
                     dst_origin,
                     region,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueCopyImageToBuffer(
    cl_command_queue  command_queue ,
    cl_mem            src_image ,
    cl_mem            dst_buffer ,
    const size_t      src_origin[3] ,
    const size_t      region[3] ,
    size_t            dst_offset ,
    cl_uint           num_events_in_wait_list ,
    const cl_event*   event_wait_list ,
    cl_event*         event)
{
    CLAPI_clEnqueueCopyImageToBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueCopyImageToBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueCopyImageToBuffer(command_queue,
                                                            src_image,
                                                            dst_buffer,
                                                            src_origin,
                                                            region,
                                                            dst_offset,
                                                            num_events_in_wait_list,
                                                            event_wait_list,
                                                            event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     src_image,
                     dst_buffer,
                     src_origin,
                     region,
                     dst_offset,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueCopyBufferToImage(
    cl_command_queue  command_queue ,
    cl_mem            src_buffer ,
    cl_mem            dst_image ,
    size_t            src_offset ,
    const size_t      dst_origin[3] ,
    const size_t      region[3] ,
    cl_uint           num_events_in_wait_list ,
    const cl_event*   event_wait_list ,
    cl_event*         event)
{
    CLAPI_clEnqueueCopyBufferToImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueCopyBufferToImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueCopyBufferToImage(command_queue,
                                                            src_buffer,
                                                            dst_image,
                                                            src_offset,
                                                            dst_origin,
                                                            region,
                                                            num_events_in_wait_list,
                                                            event_wait_list,
                                                            event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     src_buffer,
                     dst_image,
                     src_offset,
                     dst_origin,
                     region,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clEnqueueMapBuffer(
    cl_command_queue  command_queue ,
    cl_mem            buffer ,
    cl_bool           blocking_map ,
    cl_map_flags      map_flags ,
    size_t            offset ,
    size_t            cb ,
    cl_uint           num_events_in_wait_list ,
    const cl_event*   event_wait_list ,
    cl_event*         event ,
    cl_int*           errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)
    CLAPI_clEnqueueMapBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueMapBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueMapBuffer(command_queue,
                                                    buffer,
                                                    blocking_map,
                                                    map_flags,
                                                    offset,
                                                    cb,
                                                    num_events_in_wait_list,
                                                    event_wait_list,
                                                    event,
                                                    errcode_ret);
    }

    void* ret = pAPIInfo->Create(
                    command_queue,
                    buffer,
                    blocking_map,
                    map_flags,
                    offset,
                    cb,
                    num_events_in_wait_list,
                    event_wait_list,
                    event,
                    errcode_ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clEnqueueMapImage(
    cl_command_queue   command_queue ,
    cl_mem             image ,
    cl_bool            blocking_map ,
    cl_map_flags       map_flags ,
    const size_t       origin[3] ,
    const size_t       region[3] ,
    size_t*            image_row_pitch ,
    size_t*            image_slice_pitch ,
    cl_uint            num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event ,
    cl_int*            errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)
    CLAPI_clEnqueueMapImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueMapImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueMapImage(command_queue,
                                                   image,
                                                   blocking_map,
                                                   map_flags,
                                                   origin,
                                                   region,
                                                   image_row_pitch,
                                                   image_slice_pitch,
                                                   num_events_in_wait_list,
                                                   event_wait_list,
                                                   event,
                                                   errcode_ret);
    }

    void* ret = pAPIInfo->Create(
                    command_queue,
                    image,
                    blocking_map,
                    map_flags,
                    origin,
                    region,
                    image_row_pitch,
                    image_slice_pitch,
                    num_events_in_wait_list,
                    event_wait_list,
                    event,
                    errcode_ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueUnmapMemObject(
    cl_command_queue  command_queue ,
    cl_mem            memobj ,
    void*             mapped_ptr ,
    cl_uint           num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event)
{

    CLAPI_clEnqueueUnmapMemObject* pAPIInfo = new(nothrow) CLAPI_clEnqueueUnmapMemObject();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueUnmapMemObject(command_queue,
                                                         memobj,
                                                         mapped_ptr,
                                                         num_events_in_wait_list,
                                                         event_wait_list,
                                                         event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     memobj,
                     mapped_ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueNDRangeKernel(
    cl_command_queue  command_queue ,
    cl_kernel         kernel ,
    cl_uint           work_dim ,
    const size_t*     global_work_offset ,
    const size_t*     global_work_size ,
    const size_t*     local_work_size ,
    cl_uint           num_events_in_wait_list ,
    const cl_event*   event_wait_list ,
    cl_event*         event)
{
    // don't create the trace event if this is a result of a previous clEnqueueTask call
    CLAPI_clEnqueueNDRangeKernel* pAPIInfo = NULL;

    if (!CLAPIInfoManager::Instance()->CheckEnqueuedTask(kernel))
    {
        pAPIInfo = new(nothrow)CLAPI_clEnqueueNDRangeKernel();
    }

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueNDRangeKernel(command_queue,
                                                        kernel,
                                                        work_dim,
                                                        global_work_offset,
                                                        global_work_size,
                                                        local_work_size,
                                                        num_events_in_wait_list,
                                                        event_wait_list,
                                                        event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     kernel,
                     work_dim,
                     global_work_offset,
                     global_work_size,
                     local_work_size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueTask(
    cl_command_queue   command_queue ,
    cl_kernel          kernel ,
    cl_uint            num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event)
{
    CLAPIInfoManager::Instance()->AddEnqueuedTask(kernel);
    CLAPI_clEnqueueTask* pAPIInfo = new(nothrow) CLAPI_clEnqueueTask();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueTask(command_queue,
                                               kernel,
                                               num_events_in_wait_list,
                                               event_wait_list,
                                               event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     kernel,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueNativeKernel(
    cl_command_queue   command_queue ,
    void (CL_CALLBACK* user_func)(void*) ,
    void*              args ,
    size_t             cb_args ,
    cl_uint            num_mem_objects ,
    const cl_mem*      mem_list ,
    const void**       args_mem_loc ,
    cl_uint            num_events_in_wait_list ,
    const cl_event*    event_wait_list ,
    cl_event*          event)
{

    CLAPI_clEnqueueNativeKernel* pAPIInfo = new(nothrow) CLAPI_clEnqueueNativeKernel();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueNativeKernel(command_queue,
                                                       user_func,
                                                       args,
                                                       cb_args,
                                                       num_mem_objects,
                                                       mem_list,
                                                       args_mem_loc,
                                                       num_events_in_wait_list,
                                                       event_wait_list,
                                                       event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     user_func,
                     args,
                     cb_args,
                     num_mem_objects,
                     mem_list,
                     args_mem_loc,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueMarker(
    cl_command_queue     command_queue ,
    cl_event*            event)
{
    CLAPI_clEnqueueMarker* pAPIInfo = new(nothrow) CLAPI_clEnqueueMarker();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueMarker(command_queue,
                                                 event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueWaitForEvents(
    cl_command_queue  command_queue ,
    cl_uint           num_events ,
    const cl_event*   event_list)
{
    CLAPI_clEnqueueWaitForEvents* pAPIInfo = new(nothrow) CLAPI_clEnqueueWaitForEvents();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.EnqueueWaitForEvents(
                     command_queue,
                     num_events,
                     event_list);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     num_events,
                     event_list,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueBarrier(cl_command_queue  command_queue)
{
    CLAPI_clEnqueueBarrier* pAPIInfo = new(nothrow) CLAPI_clEnqueueBarrier();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.EnqueueBarrier(command_queue);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     command_queue,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromGLBuffer(
    cl_context     context,
    cl_mem_flags   flags,
    cl_GLuint      bufobj,
    int*           errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromGLBuffer* pAPIInfo = new(nothrow) CLAPI_clCreateFromGLBuffer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateFromGLBuffer(context, flags, bufobj, errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     bufobj,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromGLTexture2D(
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int*         errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromGLTexture2D* pAPIInfo = new(nothrow) CLAPI_clCreateFromGLTexture2D();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     target,
                     miplevel,
                     texture,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromGLTexture3D(
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int*         errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)
    CLAPI_clCreateFromGLTexture3D* pAPIInfo = new(nothrow) CLAPI_clCreateFromGLTexture3D();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     target,
                     miplevel,
                     texture,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromGLRenderbuffer(
    cl_context   context,
    cl_mem_flags flags,
    cl_GLuint    renderbuffer,
    cl_int*      errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)
    CLAPI_clCreateFromGLRenderbuffer* pAPIInfo = new(nothrow) CLAPI_clCreateFromGLRenderbuffer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     renderbuffer,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetGLObjectInfo(
    cl_mem                 memobj,
    cl_gl_object_type*     gl_object_type,
    cl_GLuint*             gl_object_name)
{
    CLAPI_clGetGLObjectInfo* pAPIInfo = new(nothrow) CLAPI_clGetGLObjectInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetGLObjectInfo(memobj, gl_object_type, gl_object_name);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     gl_object_type,
                     gl_object_name,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetGLTextureInfo(
    cl_mem               memobj,
    cl_gl_texture_info   param_name,
    size_t               param_value_size,
    void*                param_value,
    size_t*              param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetGLTextureInfo* pAPIInfo = new(nothrow) CLAPI_clGetGLTextureInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     memobj,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueAcquireGLObjects(
    cl_command_queue      command_queue,
    cl_uint               num_objects,
    const cl_mem*         mem_objects,
    cl_uint               num_events_in_wait_list,
    const cl_event*       event_wait_list,
    cl_event*             event)
{
    CLAPI_clEnqueueAcquireGLObjects* pAPIInfo = new(nothrow) CLAPI_clEnqueueAcquireGLObjects();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueAcquireGLObjects(command_queue,
                                                           num_objects,
                                                           mem_objects,
                                                           num_events_in_wait_list,
                                                           event_wait_list,
                                                           event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_objects,
                     mem_objects,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueReleaseGLObjects(
    cl_command_queue      command_queue,
    cl_uint               num_objects,
    const cl_mem*         mem_objects,
    cl_uint               num_events_in_wait_list,
    const cl_event*       event_wait_list,
    cl_event*             event)
{
    CLAPI_clEnqueueReleaseGLObjects* pAPIInfo = new(nothrow) CLAPI_clEnqueueReleaseGLObjects();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueReleaseGLObjects(command_queue,
                                                           num_objects,
                                                           mem_objects,
                                                           num_events_in_wait_list,
                                                           event_wait_list,
                                                           event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_objects,
                     mem_objects,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetGLContextInfoKHR(
    const cl_context_properties* properties,
    cl_gl_context_info            param_name,
    size_t                        param_value_size,
    void*                         param_value,
    size_t*                       param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetGLContextInfoKHR* pAPIInfo = new(nothrow) CLAPI_clGetGLContextInfoKHR();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetGLContextInfoKHR(properties, param_name, param_value_size, param_value, param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     properties,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_event CL_API_CALL CL_API_TRACE_clCreateEventFromGLsyncKHR(
    cl_context context,
    cl_GLsync  cl_GLsync,
    cl_int*    errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateEventFromGLsyncKHR* pAPIInfo = new(nothrow) CLAPI_clCreateEventFromGLsyncKHR();
    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_event ret = g_nextDispatchTable.CreateEventFromGLsyncKHR(context, cl_GLsync, errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     cl_GLsync,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clCreateSubDevicesEXT(
    cl_device_id     in_device,
    const cl_device_partition_property_ext* partition_properties,
    cl_uint          num_entries,
    cl_device_id*    out_devices,
    cl_uint*         num_devices)
{
    bool replaced_null_param = num_devices == NULL;

    // provide a non-null var for num_devices, only if out_devices is non-null
    // or num_entries is non-zero. If out_devices is null and num_entries is zero,
    // then leave num_devices as null, so that the real API can return CL_INVALID_VALUE
    cl_uint substituted_ret;

    if (replaced_null_param && (out_devices != NULL || num_entries != 0))
    {
        num_devices = &substituted_ret;
    }

    CLAPI_clCreateSubDevicesEXT* pAPIInfo = new(nothrow) CLAPI_clCreateSubDevicesEXT();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = ((clCreateSubDevicesEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[0]))(in_device, partition_properties,
                 num_entries, out_devices, num_devices);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     in_device,
                     partition_properties,
                     num_entries,
                     out_devices,
                     num_devices,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainDeviceEXT(
    cl_device_id     device)
{
    CLAPI_clRetainDeviceEXT* pAPIInfo = new(nothrow) CLAPI_clRetainDeviceEXT();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = ((clRetainDeviceEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[1]))(device);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     device,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseDeviceEXT(
    cl_device_id     device)
{
    CLAPI_clReleaseDeviceEXT* pAPIInfo = new(nothrow) CLAPI_clReleaseDeviceEXT();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = ((clReleaseDeviceEXT_fn)(g_nextDispatchTable._reservedForDeviceFissionEXT[2]))(device);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     device,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

#ifdef _WIN32
cl_int CL_API_CALL CL_API_TRACE_clGetDeviceIDsFromD3D10KHR(
    cl_platform_id             platform,
    cl_d3d10_device_source_khr d3d_device_source,
    void*                      d3d_object,
    cl_d3d10_device_set_khr    d3d_device_set,
    cl_uint                    num_entries,
    cl_device_id*              devices,
    cl_uint*                   num_devices)
{
    bool replaced_null_param = num_devices == NULL;

    // provide a non-null var for num_devices, only if devices is non-null
    // or num_entries is non-zero. If devices is null and num_entries is zero,
    // then leave num_devices as null, so that the real API can return CL_INVALID_VALUE
    cl_uint substituted_ret;

    if (replaced_null_param && (devices != NULL || num_entries != 0))
    {
        num_devices = &substituted_ret;
    }

    CLAPI_clGetDeviceIDsFromD3D10KHR* pAPIInfo = new(nothrow) CLAPI_clGetDeviceIDsFromD3D10KHR();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = ((clGetDeviceIDsFromD3D10KHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[0]))(
                     platform,
                     d3d_device_source,
                     d3d_object,
                     d3d_device_set,
                     num_entries,
                     devices,
                     num_devices);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     platform,
                     d3d_device_source,
                     d3d_object,
                     d3d_device_set,
                     num_entries,
                     devices,
                     num_devices,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromD3D10BufferKHR(
    cl_context     context,
    cl_mem_flags   flags,
    ID3D10Buffer* resource,
    cl_int*        errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromD3D10BufferKHR* pAPIInfo = new(nothrow) CLAPI_clCreateFromD3D10BufferKHR();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = ((clCreateFromD3D10BufferKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[1]))(
                     context,
                     flags,
                     resource,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     resource,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromD3D10Texture2DKHR(
    cl_context        context,
    cl_mem_flags      flags,
    ID3D10Texture2D* resource,
    UINT              subresource,
    cl_int*           errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromD3D10Texture2DKHR* pAPIInfo = new(nothrow) CLAPI_clCreateFromD3D10Texture2DKHR();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = ((clCreateFromD3D10Texture2DKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[2]))(
                     context,
                     flags,
                     resource,
                     subresource,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     resource,
                     subresource,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromD3D10Texture3DKHR(
    cl_context        context,
    cl_mem_flags      flags,
    ID3D10Texture3D* resource,
    UINT              subresource,
    cl_int*           errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromD3D10Texture3DKHR* pAPIInfo = new(nothrow) CLAPI_clCreateFromD3D10Texture3DKHR();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = ((clCreateFromD3D10Texture3DKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[3]))(
                     context,
                     flags,
                     resource,
                     subresource,
                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     resource,
                     subresource,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueAcquireD3D10ObjectsKHR(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem*    mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueAcquireD3D10ObjectsKHR* pAPIInfo = new(nothrow) CLAPI_clEnqueueAcquireD3D10ObjectsKHR();

    if (pAPIInfo == NULL)
    {
        return ((clEnqueueAcquireD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[4]))(command_queue,
                num_objects,
                mem_objects,
                num_events_in_wait_list,
                event_wait_list,
                event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_objects,
                     mem_objects,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueReleaseD3D10ObjectsKHR(
    cl_command_queue command_queue,
    cl_uint          num_objects,
    const cl_mem*    mem_objects,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueReleaseD3D10ObjectsKHR* pAPIInfo = new(nothrow) CLAPI_clEnqueueReleaseD3D10ObjectsKHR();

    if (pAPIInfo == NULL)
    {
        return ((clEnqueueAcquireD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[5]))(command_queue,
                num_objects,
                mem_objects,
                num_events_in_wait_list,
                event_wait_list,
                event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_objects,
                     mem_objects,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}
#endif

cl_int CL_API_CALL CL_API_TRACE_clCreateSubDevices(
    cl_device_id     in_device,
    const cl_device_partition_property* partition_properties,
    cl_uint          num_entries,
    cl_device_id*    out_devices,
    cl_uint*         num_devices)
{
    bool replaced_null_param = num_devices == NULL;

    // provide a non-null var for num_devices, only if out_devices is non-null
    // or num_entries is non-zero. If out_devices is null and num_entries is zero,
    // then leave num_devices as null, so that the real API can return CL_INVALID_VALUE
    cl_uint substituted_ret;

    if (replaced_null_param && (out_devices != NULL || num_entries != 0))
    {
        num_devices = &substituted_ret;
    }

    CLAPI_clCreateSubDevices* pAPIInfo = new(nothrow) CLAPI_clCreateSubDevices();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.CreateSubDevices(in_device, partition_properties,
                                                      num_entries, out_devices, num_devices);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     in_device,
                     partition_properties,
                     num_entries,
                     out_devices,
                     num_devices,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clRetainDevice(
    cl_device_id     device)
{
    CLAPI_clRetainDevice* pAPIInfo = new(nothrow) CLAPI_clRetainDevice();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.RetainDevice(device);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     device,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clReleaseDevice(
    cl_device_id     device)
{
    CLAPI_clReleaseDevice* pAPIInfo = new(nothrow) CLAPI_clReleaseDevice();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.ReleaseDevice(device);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     device,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateImage(
    cl_context             context,
    cl_mem_flags           flags,
    const cl_image_format* image_format,
    const cl_image_desc*   image_desc,
    void*                  host_ptr,
    cl_int*                errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateImage* pAPIInfo = new(nothrow) CLAPI_clCreateImage();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateImage(context,
                                                 flags,
                                                 image_format,
                                                 image_desc,
                                                 host_ptr,
                                                 errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     image_format,
                     image_desc,
                     host_ptr,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_program CL_API_CALL CL_API_TRACE_clCreateProgramWithBuiltInKernels(
    cl_context             context,
    cl_uint                num_devices,
    const cl_device_id*    device_list,
    const char*            kernel_names,
    cl_int*                errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateProgramWithBuiltInKernels* pAPIInfo = new(nothrow) CLAPI_clCreateProgramWithBuiltInKernels();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_program ret = g_nextDispatchTable.CreateProgramWithBuiltInKernels(context,
                                                                         num_devices,
                                                                         device_list,
                                                                         kernel_names,
                                                                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     num_devices,
                     device_list,
                     kernel_names,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clCompileProgram(
    cl_program             program,
    cl_uint                num_devices,
    const cl_device_id*    device_list,
    const char*            options,
    cl_uint                num_input_headers,
    const cl_program*      input_headers,
    const char**           header_include_names,
    void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
    void*                  user_data)
{
    CLAPI_clCompileProgram* pAPIInfo = new(nothrow) CLAPI_clCompileProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.CompileProgram(program,
                                                    num_devices,
                                                    device_list,
                                                    options,
                                                    num_input_headers,
                                                    input_headers,
                                                    header_include_names,
                                                    pfn_notify,
                                                    user_data);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     program,
                     num_devices,
                     device_list,
                     options,
                     num_input_headers,
                     input_headers,
                     header_include_names,
                     pfn_notify,
                     user_data,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_program CL_API_CALL CL_API_TRACE_clLinkProgram(
    cl_context              context,
    cl_uint                 num_devices,
    const cl_device_id*     device_list,
    const char*             options,
    cl_uint                 num_input_programs,
    const cl_program*       input_programs,
    void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
    void*                   user_data,
    cl_int*                 errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clLinkProgram* pAPIInfo = new(nothrow) CLAPI_clLinkProgram();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_program ret = g_nextDispatchTable.LinkProgram(context,
                                                     num_devices,
                                                     device_list,
                                                     options,
                                                     num_input_programs,
                                                     input_programs,
                                                     pfn_notify,
                                                     user_data,
                                                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     num_devices,
                     device_list,
                     options,
                     num_input_programs,
                     input_programs,
                     pfn_notify,
                     user_data,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clUnloadPlatformCompiler(
    cl_platform_id          platform)
{
    CLAPI_clUnloadPlatformCompiler* pAPIInfo = new(nothrow) CLAPI_clUnloadPlatformCompiler();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.UnloadPlatformCompiler(platform);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     platform,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetKernelArgInfo(
    cl_kernel              kernel,
    cl_uint                arg_index,
    cl_kernel_arg_info     param_name,
    size_t                 param_value_size,
    void*                  param_value,
    size_t*                param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetKernelArgInfo* pAPIInfo = new(nothrow) CLAPI_clGetKernelArgInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetKernelArgInfo(kernel,
                                                      arg_index,
                                                      param_name,
                                                      param_value_size,
                                                      param_value,
                                                      param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     arg_index,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueFillBuffer(
    cl_command_queue       command_queue,
    cl_mem                 buffer,
    const void*            pattern,
    size_t                 pattern_size,
    size_t                 offset,
    size_t                 size,
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    CLAPI_clEnqueueFillBuffer* pAPIInfo = new(nothrow) CLAPI_clEnqueueFillBuffer();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueFillBuffer(command_queue,
                                                     buffer,
                                                     pattern,
                                                     pattern_size,
                                                     offset,
                                                     size,
                                                     num_events_in_wait_list,
                                                     event_wait_list,
                                                     event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     buffer,
                     pattern,
                     pattern_size,
                     offset,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueFillImage(
    cl_command_queue       command_queue,
    cl_mem                 image,
    const void*            fill_color,
    const size_t           origin[3],
    const size_t           region[3],
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    CLAPI_clEnqueueFillImage* pAPIInfo = new(nothrow) CLAPI_clEnqueueFillImage();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueFillImage(command_queue,
                                                    image,
                                                    fill_color,
                                                    origin,
                                                    region,
                                                    num_events_in_wait_list,
                                                    event_wait_list,
                                                    event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     image,
                     fill_color,
                     origin,
                     region,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueMigrateMemObjects(
    cl_command_queue       command_queue,
    cl_uint                num_mem_objects,
    const cl_mem*          mem_objects,
    cl_mem_migration_flags flags,
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    CLAPI_clEnqueueMigrateMemObjects* pAPIInfo = new(nothrow) CLAPI_clEnqueueMigrateMemObjects();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueMigrateMemObjects(command_queue,
                                                            num_mem_objects,
                                                            mem_objects,
                                                            flags,
                                                            num_events_in_wait_list,
                                                            event_wait_list,
                                                            event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_mem_objects,
                     mem_objects,
                     flags,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueMarkerWithWaitList(
    cl_command_queue       command_queue,
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    CLAPI_clEnqueueMarkerWithWaitList* pAPIInfo = new(nothrow) CLAPI_clEnqueueMarkerWithWaitList();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueMarkerWithWaitList(command_queue,
                                                             num_events_in_wait_list,
                                                             event_wait_list,
                                                             event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueBarrierWithWaitList(
    cl_command_queue       command_queue,
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    CLAPI_clEnqueueBarrierWithWaitList* pAPIInfo = new(nothrow) CLAPI_clEnqueueBarrierWithWaitList();

    if (pAPIInfo == NULL)
    {
        return g_nextDispatchTable.EnqueueBarrierWithWaitList(command_queue,
                                                              num_events_in_wait_list,
                                                              event_wait_list,
                                                              event);
    }

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clGetExtensionFunctionAddressForPlatform(
    cl_platform_id         platform,
    const char*            funcname)
{
    CLAPI_clGetExtensionFunctionAddressForPlatform* pAPIInfo = new(nothrow) CLAPI_clGetExtensionFunctionAddressForPlatform();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    void* ret = g_nextDispatchTable.GetExtensionFunctionAddressForPlatform(platform, funcname);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     platform,
                     funcname,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);

    return AssignExtensionFunctionPointer(funcname, ret);
}

cl_mem CL_API_CALL CL_API_TRACE_clCreateFromGLTexture(
    cl_context             context,
    cl_mem_flags           flags,
    cl_GLenum              texture_target,
    cl_GLint               miplevel,
    cl_GLuint              texture,
    cl_int*                errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateFromGLTexture* pAPIInfo = new(nothrow) CLAPI_clCreateFromGLTexture();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreateFromGLTexture(context,
                                                         flags,
                                                         texture_target,
                                                         miplevel,
                                                         texture,
                                                         errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     texture_target,
                     miplevel,
                     texture,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clGetExtensionFunctionAddress(
    const char*            funcname)
{
    CLAPI_clGetExtensionFunctionAddress* pAPIInfo = new(nothrow) CLAPI_clGetExtensionFunctionAddress();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    void* ret = g_nextDispatchTable.GetExtensionFunctionAddress(funcname);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     funcname,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);

    return AssignExtensionFunctionPointer(funcname, ret);
}

//******OpenCL 2.0************************//

cl_command_queue CL_API_CALL CL_API_TRACE_clCreateCommandQueueWithProperties(
    cl_context                 context,
    cl_device_id               device,
    const cl_queue_properties* properties,
    cl_int*                    errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateCommandQueueWithProperties* pAPIInfo = new(nothrow) CLAPI_clCreateCommandQueueWithProperties();

    // We need to set CL_QUEUE_PROFILING_ENABLE in order to get correct GPU timestamps
    CLUtils::QueuePropertiesList propList;
    bool bUserSetProfileFlag = CLUtils::EnableQueueProfiling(properties, propList);

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_command_queue ret = g_nextDispatchTable.CreateCommandQueueWithProperties(context,
                           device,
                           propList.data(),
                           errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     device,
                     properties,
                     bUserSetProfileFlag,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_sampler CL_API_CALL CL_API_TRACE_clCreateSamplerWithProperties(
    cl_context                   context,
    const cl_sampler_properties* properties,
    cl_int*                      errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreateSamplerWithProperties* pAPIInfo = new(nothrow) CLAPI_clCreateSamplerWithProperties();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_sampler ret = g_nextDispatchTable.CreateSamplerWithProperties(context,
                                                                     properties,
                                                                     errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     properties,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clSVMAlloc(
    cl_context       context,
    cl_svm_mem_flags flags,
    size_t           size,
    cl_uint          alignment)
{
    CLAPI_clSVMAlloc* pAPIInfo = new(nothrow) CLAPI_clSVMAlloc();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    void* ret = g_nextDispatchTable.SVMAlloc(context,
                                             flags,
                                             size,
                                             alignment);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     size,
                     alignment,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void CL_API_CALL CL_API_TRACE_clSVMFree(
    cl_context context,
    void*      svm_pointer)
{
    CLAPI_clSVMFree* pAPIInfo = new(nothrow) CLAPI_clSVMFree();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    g_nextDispatchTable.SVMFree(context,
                                svm_pointer);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL);

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     svm_pointer);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
}

cl_int CL_API_CALL CL_API_TRACE_clSetKernelArgSVMPointer(
    cl_kernel   kernel,
    cl_uint     arg_index,
    const void* arg_value)
{
    CLAPI_clSetKernelArgSVMPointer* pAPIInfo = new(nothrow) CLAPI_clSetKernelArgSVMPointer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetKernelArgSVMPointer(kernel,
                                                            arg_index,
                                                            arg_value);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     arg_index,
                     arg_value,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetKernelExecInfo(
    cl_kernel           kernel,
    cl_kernel_exec_info param_name,
    size_t              param_value_size,
    const void*         param_value)
{
    CLAPI_clSetKernelExecInfo* pAPIInfo = new(nothrow) CLAPI_clSetKernelExecInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.SetKernelExecInfo(kernel,
                                                       param_name,
                                                       param_value_size,
                                                       param_value);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMFree(
    cl_command_queue command_queue,
    cl_uint          num_svm_pointers,
    void*            svm_pointers[],
    void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
    void*            user_data,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMFree* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMFree();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_svm_pointers,
                     svm_pointers,
                     pfn_free_func,
                     user_data,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMemcpy(
    cl_command_queue command_queue,
    cl_bool          blocking_copy,
    void*            dst_ptr,
    const void*      src_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMemcpy* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMemcpy();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     blocking_copy,
                     dst_ptr,
                     src_ptr,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMemFill(
    cl_command_queue command_queue,
    void*            svm_ptr,
    const void*      pattern,
    size_t           pattern_size,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMemFill* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMemFill();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     svm_ptr,
                     pattern,
                     pattern_size,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMap(
    cl_command_queue command_queue,
    cl_bool          blocking_map,
    cl_map_flags     flags,
    void*            svm_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMap* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMap();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     blocking_map,
                     flags,
                     svm_ptr,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMUnmap(
    cl_command_queue command_queue,
    void*            svm_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMUnmap* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMUnmap();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     svm_ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void* CL_API_CALL CL_API_TRACE_clSVMAllocAMD(
    cl_context       context,
    cl_svm_mem_flags flags,
    size_t           size,
    cl_uint          alignment)
{
    CLAPI_clSVMAlloc* pAPIInfo = new(nothrow) CLAPI_clSVMAlloc();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    void* ret = g_realExtensionFunctionTable.SVMAllocAMD(context,
                                                         flags,
                                                         size,
                                                         alignment);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     size,
                     alignment,
                     ret,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

void CL_API_CALL CL_API_TRACE_clSVMFreeAMD(
    cl_context context,
    void*      svm_pointer)
{
    CLAPI_clSVMFree* pAPIInfo = new(nothrow) CLAPI_clSVMFree();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    g_realExtensionFunctionTable.SVMFreeAMD(context,
                                            svm_pointer);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL);

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     svm_pointer,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
}

cl_int CL_API_CALL CL_API_TRACE_clSetKernelArgSVMPointerAMD(
    cl_kernel   kernel,
    cl_uint     arg_index,
    const void* arg_value)
{
    CLAPI_clSetKernelArgSVMPointer* pAPIInfo = new(nothrow) CLAPI_clSetKernelArgSVMPointer();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_realExtensionFunctionTable.SetKernelArgSVMPointerAMD(kernel,
                                                                        arg_index,
                                                                        arg_value);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     arg_index,
                     arg_value,
                     ret,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clSetKernelExecInfoAMD(
    cl_kernel           kernel,
    cl_kernel_exec_info param_name,
    size_t              param_value_size,
    const void*         param_value)
{
    CLAPI_clSetKernelExecInfo* pAPIInfo = new(nothrow) CLAPI_clSetKernelExecInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_realExtensionFunctionTable.SetKernelExecInfoAMD(kernel,
                                                                   param_name,
                                                                   param_value_size,
                                                                   param_value);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     kernel,
                     param_name,
                     param_value_size,
                     param_value,
                     ret,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMFreeAMD(
    cl_command_queue command_queue,
    cl_uint          num_svm_pointers,
    void*            svm_pointers[],
    void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
    void*            user_data,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMFree* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMFree();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     num_svm_pointers,
                     svm_pointers,
                     pfn_free_func,
                     user_data,
                     num_events_in_wait_list,
                     event_wait_list,
                     event,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMemcpyAMD(
    cl_command_queue command_queue,
    cl_bool          blocking_copy,
    void*            dst_ptr,
    const void*      src_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMemcpy* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMemcpy();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     blocking_copy,
                     dst_ptr,
                     src_ptr,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMemFillAMD(
    cl_command_queue command_queue,
    void*            svm_ptr,
    const void*      pattern,
    size_t           pattern_size,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMemFill* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMemFill();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     svm_ptr,
                     pattern,
                     pattern_size,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMMapAMD(
    cl_command_queue command_queue,
    cl_bool          blocking_map,
    cl_map_flags     flags,
    void*            svm_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMMap* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMMap();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     blocking_map,
                     flags,
                     svm_ptr,
                     size,
                     num_events_in_wait_list,
                     event_wait_list,
                     event, true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clEnqueueSVMUnmapAMD(
    cl_command_queue command_queue,
    void*            svm_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    CLAPI_clEnqueueSVMUnmap* pAPIInfo = new(nothrow) CLAPI_clEnqueueSVMUnmap();

    cl_int ret = pAPIInfo->Create(
                     command_queue,
                     svm_ptr,
                     num_events_in_wait_list,
                     event_wait_list,
                     event,
                     true);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_mem CL_API_CALL CL_API_TRACE_clCreatePipe(
    cl_context                context,
    cl_mem_flags              flags,
    cl_uint                   pipe_packet_size,
    cl_uint                   pipe_max_packets,
    const cl_pipe_properties* properties,
    cl_int*                   errcode_ret)
{
    REPLACE_IF_NULL(errcode_ret)

    CLAPI_clCreatePipe* pAPIInfo = new(nothrow) CLAPI_clCreatePipe();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_mem ret = g_nextDispatchTable.CreatePipe(context,
                                                flags,
                                                pipe_packet_size,
                                                pipe_max_packets,
                                                properties,
                                                errcode_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     context,
                     flags,
                     pipe_packet_size,
                     pipe_max_packets,
                     properties,
                     errcode_ret,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

cl_int CL_API_CALL CL_API_TRACE_clGetPipeInfo(
    cl_mem        pipe,
    cl_pipe_info  param_name,
    size_t        param_value_size,
    void*         param_value,
    size_t*       param_value_size_ret)
{
    bool replaced_null_param = param_value_size_ret == NULL;

    size_t substituted_ret;

    if (replaced_null_param)
    {
        param_value_size_ret = &substituted_ret;
    }

    CLAPI_clGetPipeInfo* pAPIInfo = new(nothrow) CLAPI_clGetPipeInfo();

    ULONGLONG ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(pAPIInfo);

    cl_int ret = g_nextDispatchTable.GetPipeInfo(pipe,
                                                 param_name,
                                                 param_value_size,
                                                 param_value,
                                                 param_value_size_ret);

    ULONGLONG ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(pAPIInfo);

    SpAssertRet(pAPIInfo != NULL) ret;

    pAPIInfo->Create(ullStart,
                     ullEnd,
                     pipe,
                     param_name,
                     param_value_size,
                     param_value,
                     param_value_size_ret,
                     replaced_null_param,
                     ret);

    RECORD_STACK_TRACE_FOR_API(pAPIInfo)
    CLAPIInfoManager::Instance()->AddAPIInfoEntry(pAPIInfo);
    return ret;
}

#define SetFuncPtr(func) \
    if (CLAPIInfoManager::Instance()->ShouldIntercept("cl"#func)){ \
        dispatchTable.func                 = CL_API_TRACE_cl##func; \
    }

#define SetFuncPtr1(item, func) \
    if (CLAPIInfoManager::Instance()->ShouldIntercept("cl"#func)){ \
        dispatchTable.item                 = (void*)CL_API_TRACE_cl##func; \
    }

void CreateAPITraceDispatchTable(cl_icd_dispatch_table& dispatchTable)
{
    SetFuncPtr(GetPlatformIDs)
    SetFuncPtr(GetPlatformInfo)
    SetFuncPtr(GetDeviceIDs)
    SetFuncPtr(GetDeviceInfo)
    SetFuncPtr(CreateContext)
    SetFuncPtr(CreateContextFromType)
    SetFuncPtr(RetainContext)
    SetFuncPtr(ReleaseContext)
    SetFuncPtr(GetContextInfo)
    SetFuncPtr(CreateCommandQueue)
    SetFuncPtr(RetainCommandQueue)
    SetFuncPtr(ReleaseCommandQueue)
    SetFuncPtr(GetCommandQueueInfo)
    SetFuncPtr(SetCommandQueueProperty)
    SetFuncPtr(CreateBuffer)
    SetFuncPtr(CreateSubBuffer)
    SetFuncPtr(CreateImage2D)
    SetFuncPtr(CreateImage3D)
    SetFuncPtr(RetainMemObject)
    SetFuncPtr(ReleaseMemObject)
    SetFuncPtr(GetSupportedImageFormats)
    SetFuncPtr(GetMemObjectInfo)
    SetFuncPtr(GetImageInfo)
    SetFuncPtr(SetMemObjectDestructorCallback)
    SetFuncPtr(CreateSampler)
    SetFuncPtr(RetainSampler)
    SetFuncPtr(ReleaseSampler)
    SetFuncPtr(GetSamplerInfo)
    SetFuncPtr(CreateProgramWithSource)
    SetFuncPtr(CreateProgramWithBinary)
    SetFuncPtr(RetainProgram)
    SetFuncPtr(ReleaseProgram)
    SetFuncPtr(BuildProgram)
    SetFuncPtr(UnloadCompiler)
    SetFuncPtr(GetProgramInfo)
    SetFuncPtr(GetProgramBuildInfo)
    SetFuncPtr(CreateKernel)
    SetFuncPtr(CreateKernelsInProgram)
    SetFuncPtr(RetainKernel)
    SetFuncPtr(ReleaseKernel)
    SetFuncPtr(SetKernelArg)
    SetFuncPtr(GetKernelInfo)
    SetFuncPtr(GetKernelWorkGroupInfo)
    SetFuncPtr(WaitForEvents)
    SetFuncPtr(GetEventInfo)
    SetFuncPtr(CreateUserEvent)
    SetFuncPtr(RetainEvent)
    SetFuncPtr(ReleaseEvent)
    SetFuncPtr(SetUserEventStatus)
    SetFuncPtr(SetEventCallback)
    SetFuncPtr(GetEventProfilingInfo)
    SetFuncPtr(Flush)
    SetFuncPtr(Finish)
    SetFuncPtr(EnqueueReadBuffer)
    SetFuncPtr(EnqueueReadBufferRect)
    SetFuncPtr(EnqueueWriteBuffer)
    SetFuncPtr(EnqueueWriteBufferRect)
    SetFuncPtr(EnqueueCopyBuffer)
    SetFuncPtr(EnqueueCopyBufferRect)
    SetFuncPtr(EnqueueReadImage)
    SetFuncPtr(EnqueueWriteImage)
    SetFuncPtr(EnqueueCopyImage)
    SetFuncPtr(EnqueueCopyImageToBuffer)
    SetFuncPtr(EnqueueCopyBufferToImage)
    SetFuncPtr(EnqueueMapBuffer)
    SetFuncPtr(EnqueueMapImage)
    SetFuncPtr(EnqueueUnmapMemObject)
    SetFuncPtr(EnqueueNDRangeKernel)
    SetFuncPtr(EnqueueTask)
    SetFuncPtr(EnqueueNativeKernel)
    SetFuncPtr(EnqueueMarker)
    SetFuncPtr(EnqueueWaitForEvents)
    SetFuncPtr(EnqueueBarrier)

    SetFuncPtr(CreateFromGLBuffer)
    SetFuncPtr(CreateFromGLTexture2D)
    SetFuncPtr(CreateFromGLTexture3D)
    SetFuncPtr(CreateFromGLRenderbuffer)
    SetFuncPtr(GetGLObjectInfo)
    SetFuncPtr(GetGLTextureInfo)
    SetFuncPtr(EnqueueAcquireGLObjects)
    SetFuncPtr(EnqueueReleaseGLObjects)
    SetFuncPtr(GetGLContextInfoKHR)

    SetFuncPtr(CreateEventFromGLsyncKHR)
    SetFuncPtr(CreateSubDevices)
    SetFuncPtr(RetainDevice)
    SetFuncPtr(ReleaseDevice)
    SetFuncPtr(CreateImage)
    SetFuncPtr(CreateProgramWithBuiltInKernels)
    SetFuncPtr(CompileProgram)
    SetFuncPtr(LinkProgram)
    SetFuncPtr(UnloadPlatformCompiler)
    SetFuncPtr(GetKernelArgInfo)
    SetFuncPtr(EnqueueFillBuffer)
    SetFuncPtr(EnqueueFillImage)
    SetFuncPtr(EnqueueMigrateMemObjects)
    SetFuncPtr(EnqueueMarkerWithWaitList)
    SetFuncPtr(EnqueueBarrierWithWaitList)
    SetFuncPtr(GetExtensionFunctionAddressForPlatform)
    SetFuncPtr(CreateFromGLTexture)

    SetFuncPtr(GetExtensionFunctionAddress)

#ifdef OPENCL_FISSION_EXT_SUPPORT
    SetFuncPtr1(_reservedForDeviceFissionEXT[0], CreateSubDevicesEXT)
    SetFuncPtr1(_reservedForDeviceFissionEXT[1], RetainDeviceEXT)
    SetFuncPtr1(_reservedForDeviceFissionEXT[2], ReleaseDeviceEXT)
#endif

#ifdef _WIN32
    SetFuncPtr1(_reservedForD3D10KHR[0], GetDeviceIDsFromD3D10KHR)
    SetFuncPtr1(_reservedForD3D10KHR[1], CreateFromD3D10BufferKHR)
    SetFuncPtr1(_reservedForD3D10KHR[2], CreateFromD3D10Texture2DKHR)
    SetFuncPtr1(_reservedForD3D10KHR[3], CreateFromD3D10Texture3DKHR)
    SetFuncPtr1(_reservedForD3D10KHR[4], EnqueueAcquireD3D10ObjectsKHR)
    SetFuncPtr1(_reservedForD3D10KHR[5], EnqueueReleaseD3D10ObjectsKHR)
#endif

    // OpenCL 2.0
    SetFuncPtr(CreateCommandQueueWithProperties)
    SetFuncPtr(CreatePipe)
    SetFuncPtr(GetPipeInfo)
    SetFuncPtr(SVMAlloc)
    SetFuncPtr(SVMFree)
    SetFuncPtr(EnqueueSVMFree)
    SetFuncPtr(EnqueueSVMMemcpy)
    SetFuncPtr(EnqueueSVMMemFill)
    SetFuncPtr(EnqueueSVMMap)
    SetFuncPtr(EnqueueSVMUnmap)
    SetFuncPtr(CreateSamplerWithProperties)
    SetFuncPtr(SetKernelArgSVMPointer)
    SetFuncPtr(SetKernelExecInfo)
}

void* AssignExtensionFunctionPointer(const char* pFuncName, void* pRealFuncPtr)
{
    void* pRetVal = pRealFuncPtr;

    if (pRealFuncPtr != NULL)
    {
        CL_FUNC_TYPE funcType = InitExtensionFunction(pFuncName, pRetVal);

        switch (funcType)
        {
            case CL_FUNC_TYPE_clSVMAllocAMD:
                pRetVal = (void*)CL_API_TRACE_clSVMAllocAMD;
                break;

            case CL_FUNC_TYPE_clSVMFreeAMD:
                pRetVal = (void*)CL_API_TRACE_clSVMFreeAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMFreeAMD:
                pRetVal = (void*)CL_API_TRACE_clEnqueueSVMFreeAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD:
                pRetVal = (void*)CL_API_TRACE_clEnqueueSVMMemcpyAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMemFillAMD:
                pRetVal = (void*)CL_API_TRACE_clEnqueueSVMMemFillAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMMapAMD:
                pRetVal = (void*)CL_API_TRACE_clEnqueueSVMMapAMD;
                break;

            case CL_FUNC_TYPE_clEnqueueSVMUnmapAMD:
                pRetVal = (void*)CL_API_TRACE_clEnqueueSVMUnmapAMD;
                break;

            case CL_FUNC_TYPE_clSetKernelArgSVMPointerAMD:
                pRetVal = (void*)CL_API_TRACE_clSetKernelArgSVMPointerAMD;
                break;

            case CL_FUNC_TYPE_clSetKernelExecInfoAMD:
                pRetVal = (void*)CL_API_TRACE_clSetKernelExecInfoAMD;
                break;

            default:
                break;

        }
    }

    return pRetVal;
};

