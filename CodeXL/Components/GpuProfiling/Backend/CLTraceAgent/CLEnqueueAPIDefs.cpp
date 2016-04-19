//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file implement all enqueue cmd create functions
//==============================================================================

#include <iomanip>
#include "CLAPIInfoManager.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Logger.h"
#include "DeviceInfoUtils.h"
#include "PMCSamplerManager.h"

using namespace std;

static void CopyEventList(const cl_event* event_wait_list,
                          cl_uint   num_events_in_wait_list,
                          std::vector<cl_event>& events)
{
    if (event_wait_list == NULL)
    {
        return;
    }

    for (cl_uint i = 0; i < num_events_in_wait_list; i++)
    {
        events.push_back(event_wait_list[i]);
    }
}

cl_int CLAPI_clEnqueueReadBuffer::Create(
    cl_command_queue  command_queue,
    cl_mem   buffer,
    cl_bool  blocking_read,
    size_t   offset,
    size_t   cb,
    void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueReadBuffer(
                   command_queue,
                   buffer,
                   blocking_read,
                   offset,
                   cb,
                   ptr,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueReadBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_blocking_read = blocking_read;
    m_offset = offset;
    m_cb = cb;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueReadBufferRect::Create(
    cl_command_queue  command_queue,
    cl_mem   buffer,
    cl_bool  blocking_read,
    const size_t   buffer_offset[3],
    const size_t   host_offset[3],
    const size_t   region[3],
    size_t   buffer_row_pitch,
    size_t   buffer_slice_pitch,
    size_t   host_row_pitch,
    size_t   host_slice_pitch,
    void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{

    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueReadBufferRect(
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
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueReadBufferRect;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_blocking_read = blocking_read;

    m_buffer_offset_null = buffer_offset == NULL;

    if (!m_buffer_offset_null)
    {
        m_buffer_offset[ 0 ] = buffer_offset[ 0 ];
        m_buffer_offset[ 1 ] = buffer_offset[ 1 ];
        m_buffer_offset[ 2 ] = buffer_offset[ 2 ];
    }

    m_host_offset_null = host_offset == NULL;

    if (!m_host_offset_null)
    {
        m_host_offset[ 0 ] = host_offset[ 0 ];
        m_host_offset[ 1 ] = host_offset[ 1 ];
        m_host_offset[ 2 ] = host_offset[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_buffer_row_pitch = buffer_row_pitch;
    m_buffer_slice_pitch = buffer_slice_pitch;
    m_host_row_pitch = host_row_pitch;
    m_host_slice_pitch = host_slice_pitch;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueWriteBuffer::Create(
    cl_command_queue  command_queue,
    cl_mem   buffer,
    cl_bool  blocking_write,
    size_t   offset,
    size_t   cb,
    const void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueWriteBuffer(
                   command_queue,
                   buffer,
                   blocking_write,
                   offset,
                   cb,
                   ptr,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_type = CL_FUNC_TYPE_clEnqueueWriteBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_blocking_write = blocking_write;
    m_offset = offset;
    m_cb = cb;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueWriteBufferRect::Create(
    cl_command_queue  command_queue,
    cl_mem   buffer,
    cl_bool  blocking_read,
    const size_t   buffer_offset[3],
    const size_t   host_offset[3],
    const size_t   region[3],
    size_t   buffer_row_pitch,
    size_t   buffer_slice_pitch,
    size_t   host_row_pitch,
    size_t   host_slice_pitch,
    const void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueWriteBufferRect(
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
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueWriteBufferRect;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_blocking_read = blocking_read;

    m_buffer_offset_null = buffer_offset == NULL;

    if (!m_buffer_offset_null)
    {
        m_buffer_offset[ 0 ] = buffer_offset[ 0 ];
        m_buffer_offset[ 1 ] = buffer_offset[ 1 ];
        m_buffer_offset[ 2 ] = buffer_offset[ 2 ];
    }

    m_host_offset_null = host_offset == NULL;

    if (!m_host_offset_null)
    {
        m_host_offset[ 0 ] = host_offset[ 0 ];
        m_host_offset[ 1 ] = host_offset[ 1 ];
        m_host_offset[ 2 ] = host_offset[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_buffer_row_pitch = buffer_row_pitch;
    m_buffer_slice_pitch = buffer_slice_pitch;
    m_host_row_pitch = host_row_pitch;
    m_host_slice_pitch = host_slice_pitch;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueCopyBuffer::Create(
    cl_command_queue  command_queue,
    cl_mem   src_buffer,
    cl_mem   dst_buffer,
    size_t   src_offset,
    size_t   dst_offset,
    size_t   cb,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueCopyBuffer(
                   command_queue,
                   src_buffer,
                   dst_buffer,
                   src_offset,
                   dst_offset,
                   cb,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueCopyBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_src_buffer = src_buffer;
    m_dst_buffer = dst_buffer;
    m_src_offset = src_offset;
    m_dst_offset = dst_offset;
    m_cb = cb;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueCopyBufferRect::Create(
    cl_command_queue  command_queue,
    cl_mem   src_buffer,
    cl_mem   dst_buffer,
    const size_t   src_origin[3],
    const size_t   dst_origin[3],
    const size_t   region[3],
    size_t   src_row_pitch,
    size_t   src_slice_pitch,
    size_t   dst_row_pitch,
    size_t   dst_slice_pitch,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueCopyBufferRect(
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
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueCopyBufferRect;
    m_command_queue = command_queue;
    GetContextInfo();

    m_src_buffer = src_buffer;
    m_dst_buffer = dst_buffer;

    m_src_origin_null = src_origin == NULL;

    if (!m_src_origin_null)
    {
        m_src_origin[ 0 ] = src_origin[ 0 ];
        m_src_origin[ 1 ] = src_origin[ 1 ];
        m_src_origin[ 2 ] = src_origin[ 2 ];
    }

    m_dst_origin_null = dst_origin == NULL;

    if (!m_dst_origin_null)
    {
        m_dst_origin[ 0 ] = dst_origin[ 0 ];
        m_dst_origin[ 1 ] = dst_origin[ 1 ];
        m_dst_origin[ 2 ] = dst_origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_src_row_pitch = src_row_pitch;
    m_src_slice_pitch = src_slice_pitch;
    m_dst_row_pitch = dst_row_pitch;
    m_dst_slice_pitch = dst_slice_pitch;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueReadImage::Create(
    cl_command_queue  command_queue,
    cl_mem   image,
    cl_bool  blocking_read,
    const size_t   origin[3],
    const size_t   region[3],
    size_t   row_pitch,
    size_t   slice_pitch,
    void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{

    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueReadImage(
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
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueReadImage;
    m_command_queue = command_queue;
    GetContextInfo();
    m_image = image;
    m_blocking_read = blocking_read;

    m_origin_null = origin == NULL;

    if (!m_origin_null)
    {
        m_origin[ 0 ] = origin[ 0 ];
        m_origin[ 1 ] = origin[ 1 ];
        m_origin[ 2 ] = origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_row_pitch = row_pitch;
    m_slice_pitch = slice_pitch;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueWriteImage::Create(
    cl_command_queue  command_queue,
    cl_mem   image,
    cl_bool  blocking_write,
    const size_t   origin[3],
    const size_t   region[3],
    size_t   input_row_pitch,
    size_t   input_slice_pitch,
    const void* ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{

    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueWriteImage(
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
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueWriteImage;
    m_command_queue = command_queue;
    GetContextInfo();
    m_image = image;
    m_blocking_write = blocking_write;

    m_origin_null = origin == NULL;

    if (!m_origin_null)
    {
        m_origin[ 0 ] = origin[ 0 ];
        m_origin[ 1 ] = origin[ 1 ];
        m_origin[ 2 ] = origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_input_row_pitch = input_row_pitch;
    m_input_slice_pitch = input_slice_pitch;
    m_ptr = ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueCopyImage::Create(
    cl_command_queue  command_queue,
    cl_mem   src_image,
    cl_mem   dst_image,
    const size_t   src_origin[3],
    const size_t   dst_origin[3],
    const size_t   region[3],
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueCopyImage(
                   command_queue,
                   src_image,
                   dst_image,
                   src_origin,
                   dst_origin,
                   region,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueCopyImage;
    m_command_queue = command_queue;

    GetContextInfo();

    m_src_image = src_image;
    m_dst_image = dst_image;

    m_src_origin_null = src_origin == NULL;

    if (!m_src_origin_null)
    {
        m_src_origin[ 0 ] = src_origin[ 0 ];
        m_src_origin[ 1 ] = src_origin[ 1 ];
        m_src_origin[ 2 ] = src_origin[ 2 ];
    }

    m_dst_origin_null = dst_origin == NULL;

    if (!m_dst_origin_null)
    {
        m_dst_origin[ 0 ] = dst_origin[ 0 ];
        m_dst_origin[ 1 ] = dst_origin[ 1 ];
        m_dst_origin[ 2 ] = dst_origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(dst_image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueCopyImageToBuffer::Create(
    cl_command_queue  command_queue,
    cl_mem   src_image,
    cl_mem   dst_buffer,
    const size_t   src_origin[3],
    const size_t   region[3],
    size_t   dst_offset,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{

    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueCopyImageToBuffer(
                   command_queue,
                   src_image,
                   dst_buffer,
                   src_origin,
                   region,
                   dst_offset,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueCopyImageToBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_src_image = src_image;
    m_dst_buffer = dst_buffer;

    m_src_origin_null = src_origin == NULL;

    if (!m_src_origin_null)
    {
        m_src_origin[ 0 ] = src_origin[ 0 ];
        m_src_origin[ 1 ] = src_origin[ 1 ];
        m_src_origin[ 2 ] = src_origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_dst_offset = dst_offset;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(src_image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueCopyBufferToImage::Create(
    cl_command_queue  command_queue,
    cl_mem   src_buffer,
    cl_mem   dst_image,
    size_t   src_offset,
    const size_t   dst_origin[3],
    const size_t   region[3],
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueCopyBufferToImage(
                   command_queue,
                   src_buffer,
                   dst_image,
                   src_offset,
                   dst_origin,
                   region,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueCopyBufferToImage;
    m_command_queue = command_queue;
    GetContextInfo();
    m_src_buffer = src_buffer;
    m_dst_image = dst_image;
    m_src_offset = src_offset;

    m_dst_origin_null = dst_origin == NULL;

    if (!m_dst_origin_null)
    {
        m_dst_origin[ 0 ] = dst_origin[ 0 ];
        m_dst_origin[ 1 ] = dst_origin[ 1 ];
        m_dst_origin[ 2 ] = dst_origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(dst_image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

bool CLAPI_clEnqueueMapMemObj::GetMemDeviceInfo(cl_mem buffer)
{
    if (m_command_queue == NULL || buffer == NULL)
    {
        return false;
    }

    // get mem flag
    cl_int iRet = g_realDispatchTable.GetMemObjectInfo(buffer, CL_MEM_FLAGS, sizeof(cl_mem_flags), &m_memFlags, NULL);

    if (iRet != CL_SUCCESS)
    {
        m_deviceType = DT_Unknown;
        return false;
    }

    cl_context ctx = m_context;

    // get device type
    // normally when this function is called, we should already get device name from
    // GetContextInfo()
    if (!m_strDeviceName.empty())
    {
        if (m_strDeviceName == "CPU_Device")
        {
            m_deviceType = DT_CPU;
        }
        else
        {
            bool bIsAPU = false;

            if (AMDTDeviceInfoUtils::Instance()->IsAPU(m_strDeviceName.c_str(), bIsAPU))
            {
                if (bIsAPU)
                {
                    m_deviceType = DT_APU;
                }
                else
                {
                    m_deviceType = DT_DiscreteGPU;
                }
            }
            else
            {
                m_deviceType = DT_Unknown;
                return false;
            }
        }
    }
    else
    {
        // fall back path
        cl_device_id deviceID;
        iRet = g_realDispatchTable.GetCommandQueueInfo(m_command_queue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &deviceID, NULL);

        if (iRet != CL_SUCCESS)
        {
            m_deviceType = DT_Unknown;
            return false;
        }

        cl_device_type dtype;
        iRet = g_realDispatchTable.GetDeviceInfo(deviceID, CL_DEVICE_TYPE, sizeof(cl_device_type), &dtype, NULL);

        if (iRet != CL_SUCCESS)
        {
            m_deviceType = DT_Unknown;
            return false;
        }

        if (dtype == CL_DEVICE_TYPE_CPU)
        {
            m_deviceType = DT_CPU;
        }
        else
        {
            char szDeviceName[SP_MAX_PATH];
            iRet = g_realDispatchTable.GetDeviceInfo(deviceID, CL_DEVICE_NAME, SP_MAX_PATH, szDeviceName, NULL);

            if (iRet != CL_SUCCESS)
            {
                m_deviceType = DT_Unknown;
                return false;
            }

            bool bIsAPU = false;

            if (AMDTDeviceInfoUtils::Instance()->IsAPU(szDeviceName, bIsAPU))
            {
                if (bIsAPU)
                {
                    m_deviceType = DT_APU;
                }
                else
                {
                    m_deviceType = DT_DiscreteGPU;
                }
            }
            else
            {
                return false;
            }
        }
    }

    if (m_deviceType == DT_CPU)
    {
        // If current device is CPU, we want to know if current context has any non-CPU devices
        if (ctx == NULL)
        {
            // fall back
            iRet = g_realDispatchTable.GetCommandQueueInfo(m_command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &ctx, NULL);

            if (iRet != CL_SUCCESS)
            {
                m_deviceType = DT_Unknown;
                return false;
            }
        }

        // iterate all cmd queues for the current context
        size_t nDeviceSize = 0;
        iRet = g_realDispatchTable.GetContextInfo(ctx, CL_CONTEXT_DEVICES, 0, NULL, &nDeviceSize);

        if (iRet != CL_SUCCESS || nDeviceSize == 0)
        {
            m_deviceType = DT_Unknown;
            return false;
        }

        size_t nDevice = nDeviceSize / sizeof(cl_device_id);
        cl_device_id* devices = new(std::nothrow) cl_device_id[nDevice];
        iRet = g_realDispatchTable.GetContextInfo(ctx, CL_CONTEXT_DEVICES, nDeviceSize, devices, NULL);

        if (iRet != CL_SUCCESS)
        {
            m_deviceType = DT_Unknown;
            delete[] devices;
            return false;
        }

        if (nDevice > 1)
        {
            m_bCPUOnlyContext = true;

            // more than one device, find if there is a non-CPU device
            for (size_t i = 0; i < nDevice; ++i)
            {
                cl_device_type dtype;
                iRet = g_realDispatchTable.GetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &dtype, NULL);

                if (dtype != CL_DEVICE_TYPE_CPU)
                {
                    m_bCPUOnlyContext = false;
                    break;
                }
            }

            delete[] devices;
        }
    }

    return true;
}

bool CLAPI_clEnqueueMapMemObj::GetLocation(MemLocation& allocLocation, MemLocation& mapLocation, bool& bZeroCopy, bool bImage) const
{
#ifndef _WIN32
    // Linux
    bool bVM = false;
    bool bIsGCN = false;
    AMDTDeviceInfoUtils::Instance()->IsGCN(m_strDeviceName.c_str(), bIsGCN);

    if (bIsGCN)
    {
        // SI and CI supports VM on Linux
        bVM = true;
    }

#else
    bool bVM = true;
#endif

    const size_t DATA_THRESHOLD = 33554432;   // 32MB

    switch (m_deviceType)
    {
        case DT_APU:

            if (m_memFlags & CL_MEM_USE_HOST_PTR)
            {
                allocLocation = ML_DeviceVisibleHostMem;
                mapLocation = ML_PinnedHostMem;
                bZeroCopy = false;
            }
            else if (m_memFlags & CL_MEM_ALLOC_HOST_PTR)
            {
                if (!bVM || bImage)
                {
                    // no VM
                    allocLocation = ML_DeviceVisibleHostMem;
                    mapLocation = ML_PinnedHostMem;
                    bZeroCopy = false;
                }
                else
                {
                    allocLocation = ML_PinnedHostMem;
                    mapLocation = ML_PinnedHostMem;
                    bZeroCopy = true;
                }
            }
            else if (m_memFlags & CL_MEM_USE_PERSISTENT_MEM_AMD)
            {
                if (!bVM)
                {
                    allocLocation = ML_DeviceVisibleHostMem;

                    if (GetDataSize() > DATA_THRESHOLD)
                    {
                        mapLocation = ML_HostMem;
                    }
                    else
                    {
                        mapLocation = ML_PinnedHostMem;
                    }

                    bZeroCopy = false;
                }
                else
                {
                    allocLocation = ML_DeviceVisibleHostMem;
                    mapLocation = ML_DeviceVisibleHostMem;
                    bZeroCopy = true;
                }
            }
            else
            {
                allocLocation = ML_DeviceVisibleHostMem;

                if (GetDataSize() > DATA_THRESHOLD)
                {
                    mapLocation = ML_HostMem;
                }
                else
                {
                    mapLocation = ML_PinnedHostMem;
                }

                bZeroCopy = false;
            }

            break;

        case DT_DiscreteGPU:

            if (m_memFlags & CL_MEM_USE_HOST_PTR)
            {
                allocLocation = ML_DeviceMem;
                mapLocation = ML_PinnedHostMem;
                bZeroCopy = false;
            }
            else if (m_memFlags & CL_MEM_ALLOC_HOST_PTR)
            {
                if (!bVM || bImage)
                {
                    // no VM
                    allocLocation = ML_DeviceMem;
                    mapLocation = ML_PinnedHostMem;
                    bZeroCopy = false;
                }
                else
                {
                    allocLocation = ML_PinnedHostMem;
                    mapLocation = ML_PinnedHostMem;
                    bZeroCopy = true;
                }
            }
            else if (m_memFlags & CL_MEM_USE_PERSISTENT_MEM_AMD)
            {
                if (!bVM)
                {
                    allocLocation = ML_DeviceMem;

                    if (GetDataSize() > DATA_THRESHOLD)
                    {
                        mapLocation = ML_HostMem;
                    }
                    else
                    {
                        mapLocation = ML_PinnedHostMem;
                    }

                    bZeroCopy = false;
                }
                else
                {
                    allocLocation = ML_HostVisibleDeviceMem;
                    mapLocation = ML_HostVisibleDeviceMem;
                    bZeroCopy = true;
                }
            }
            else
            {
                allocLocation = ML_DeviceMem;

                if (GetDataSize() > DATA_THRESHOLD)
                {
                    mapLocation = ML_HostMem;
                }
                else
                {
                    mapLocation = ML_PinnedHostMem;
                }

                bZeroCopy = false;
            }

            break;

        case DT_CPU:

            if (m_memFlags & CL_MEM_USE_HOST_PTR)
            {
                if (m_bCPUOnlyContext)
                {
                    allocLocation = ML_HostMem;
                    mapLocation = ML_HostMem;
                }
                else
                {
                    allocLocation = ML_PinnedHostMem;
                    mapLocation = ML_PinnedHostMem;
                }

                bZeroCopy = true;
            }
            else if (m_memFlags & CL_MEM_ALLOC_HOST_PTR)
            {
                if (m_bCPUOnlyContext)
                {
                    allocLocation = ML_HostMem;
                    mapLocation = ML_HostMem;
                }
                else
                {
                    allocLocation = ML_PinnedHostMem;
                    mapLocation = ML_PinnedHostMem;
                }

                bZeroCopy = true;
            }
            else if (m_memFlags & CL_MEM_USE_PERSISTENT_MEM_AMD)
            {
                if (!bVM || bImage)
                {
                    if (GetDataSize() > DATA_THRESHOLD)
                    {
                        allocLocation = ML_HostMem;
                        mapLocation = ML_HostMem;
                    }
                    else
                    {
                        allocLocation = ML_PinnedHostMem;
                        mapLocation = ML_PinnedHostMem;
                    }

                    bZeroCopy = true;
                }
                else
                {
                    allocLocation = ML_HostMem;
                    mapLocation = ML_HostMem;
                    bZeroCopy = true;
                }
            }
            else
            {
                if (GetDataSize() > DATA_THRESHOLD)
                {
                    allocLocation = ML_HostMem;
                    mapLocation = ML_HostMem;
                }
                else
                {
                    allocLocation = ML_PinnedHostMem;
                    mapLocation = ML_PinnedHostMem;
                }

                bZeroCopy = true;
            }

            break;

        default:
            return false;
            break;
    }

    return true;
}


void* CLAPI_clEnqueueMapBuffer::Create(
    cl_command_queue  command_queue,
    cl_mem   buffer,
    cl_bool  blocking_map,
    cl_map_flags   map_flags,
    size_t   offset,
    size_t   cb,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event,
    cl_int*  errcode_ret)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueMapBuffer(
                   command_queue,
                   buffer,
                   blocking_map,
                   map_flags,
                   offset,
                   cb,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent,
                   errcode_ret);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueMapBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_blocking_map = blocking_map;
    m_map_flags = map_flags;
    m_offset = offset;
    m_cb = cb;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    m_errcode_ret = errcode_ret;

    if (errcode_ret != NULL)
    {
        m_errcode_retVal = *errcode_ret;
    }
    else
    {
        m_errcode_retVal = 0;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);

        GetMemDeviceInfo(buffer);
    }

    return m_retVal;
}

void* CLAPI_clEnqueueMapImage::Create(
    cl_command_queue  command_queue,
    cl_mem   image,
    cl_bool  blocking_map,
    cl_map_flags   map_flags,
    const size_t   origin[3],
    const size_t   region[3],
    size_t*  image_row_pitch,
    size_t*  image_slice_pitch,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event,
    cl_int*  errcode_ret)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueMapImage(
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
                   pTmpEvent,
                   errcode_ret);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueMapImage;
    m_command_queue = command_queue;
    GetContextInfo();
    m_image = image;
    m_blocking_map = blocking_map;
    m_map_flags = map_flags;

    m_origin_null = origin == NULL;

    if (!m_origin_null)
    {
        m_origin[ 0 ] = origin[ 0 ];
        m_origin[ 1 ] = origin[ 1 ];
        m_origin[ 2 ] = origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_image_row_pitch = image_row_pitch;

    if (image_row_pitch != NULL)
    {
        m_image_row_pitchVal = *image_row_pitch;
    }
    else
    {
        m_image_row_pitchVal = 0;
    }

    m_image_slice_pitch = image_slice_pitch;

    if (image_slice_pitch != NULL)
    {
        m_image_slice_pitchVal = *image_slice_pitch;
    }
    else
    {
        m_image_slice_pitchVal = 0;
    }

    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    m_errcode_ret = errcode_ret;

    if (errcode_ret != NULL)
    {
        m_errcode_retVal = *errcode_ret;
    }
    else
    {
        m_errcode_retVal = 0;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
        GetMemDeviceInfo(image);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(m_image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueUnmapMemObject::Create(
    cl_command_queue  command_queue,
    cl_mem   memobj,
    void* mapped_ptr,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueUnmapMemObject(
                   command_queue,
                   memobj,
                   mapped_ptr,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueUnmapMemObject;
    m_command_queue = command_queue;
    GetContextInfo();
    m_memobj = memobj;
    m_mapped_ptr = mapped_ptr;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueNDRangeKernel::Create(
    cl_command_queue  command_queue,
    cl_kernel   kernel,
    cl_uint  work_dim,
    const size_t*  global_work_offset,
    const size_t*  global_work_size,
    const size_t*  local_work_size,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueNDRangeKernel(
                   command_queue,
                   kernel,
                   work_dim,
                   global_work_offset,
                   global_work_size,
                   local_work_size,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueNDRangeKernel;
    m_command_queue = command_queue;
    GetContextInfo();
    m_kernel = kernel;
    m_work_dim = work_dim;
    //m_global_work_offset = global_work_offset;
    //m_global_work_size = global_work_size;
    //m_local_work_size = local_work_size;

    DeepCopyArray(&m_global_work_offset, global_work_offset, m_work_dim);
    DeepCopyArray(&m_global_work_size, global_work_size, m_work_dim);

    if (local_work_size != NULL)
    {
        DeepCopyArray(&m_local_work_size, local_work_size, m_work_dim);
    }
    else
    {
        m_local_work_size = NULL;
    }

    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // get kernel name
    m_strKernelName = CLAPIInfoManager::Instance()->GetKernelName(m_kernel);

    return m_retVal;
}

bool CLAPI_clEnqueueNDRangeKernel::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueAPIBase::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    // print out kernel name and kernel handle
    const cl_kernel kernel = GetKernel();
    sout << setw(25) << StringUtils::ToHexString(kernel);
    sout << GetKernelName();

    // print out work group size, global size
    sout << std::dec;
    const size_t* globalSize = GetGlobalWorkSize();
    const size_t* localSize = GetLocalWorkSize();
    cl_uint workDim = GetWorkDim();
    sout << "      {";

    for (cl_uint m = 0; m < workDim; m++)
    {
        if (m == workDim - 1)
        {
            sout << globalSize[m];
        }
        else
        {
            sout << globalSize[m] << ",";
        }
    }

    sout << "}     ";

    if (localSize == NULL)
    {
        // local size can be null
        sout << "{NULL}        ";
    }
    else
    {
        sout << "{";

        for (cl_uint m = 0; m < workDim; m++)
        {
            if (m == workDim - 1)
            {
                sout << localSize[m];
            }
            else
            {
                sout << localSize[m] << ",";
            }
        }

        sout << "}        ";
    }

    // switch back the dec mode
    sout << std::dec;

    return true;
}

cl_int CLAPI_clEnqueueTask::Create(
    cl_command_queue  command_queue,
    cl_kernel   kernel,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueTask(
                   command_queue,
                   kernel,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueTask;
    m_command_queue = command_queue;
    GetContextInfo();
    m_kernel = kernel;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

bool CLAPI_clEnqueueTask::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueAPIBase::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    // print out kernel name and kernel handle
    const cl_kernel kernel = GetKernel();
    std::string strKernelName = CLAPIInfoManager::Instance()->GetKernelName(kernel);
    sout << setw(25) << StringUtils::ToHexString(kernel);
    sout << strKernelName;

    // output {1} for both global and local work size
    sout << "      {1}     {1}        ";

    // switch back the dec mode
    sout << std::dec;

    return true;
}

cl_int CLAPI_clEnqueueNativeKernel::Create(
    cl_command_queue  command_queue,
    void (CL_CALLBACK* user_func)(void*),
    void* args,
    size_t   cb_args,
    cl_uint  num_mem_objects,
    const cl_mem*  mem_list,
    const void**   args_mem_loc,
    cl_uint  num_events_in_wait_list,
    const cl_event*   event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueNativeKernel(
                   command_queue,
                   user_func,
                   args,
                   cb_args,
                   num_mem_objects,
                   mem_list,
                   args_mem_loc,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueNativeKernel;
    m_command_queue = command_queue;
    GetContextInfo();
    m_user_func = user_func;
    m_args = args;
    m_cb_args = cb_args;
    m_num_mem_objects = num_mem_objects;
    //m_mem_list = mem_list;
    DeepCopyArray(&m_mem_list, mem_list, num_mem_objects);
    m_args_mem_loc = args_mem_loc;
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

bool CLAPI_clEnqueueNativeKernel::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    bool bRet = CLEnqueueAPIBase::WriteTimestampEntry(sout, bTimeout);

    if (!bRet)
    {
        return bRet;
    }

    if (!GetAPISucceeded())
    {
        return true;
    }

    sout << setw(25) << StringUtils::ToHexString(m_user_func);

    // switch back the dec mode
    sout << std::dec;

    return true;
}

cl_int CLAPI_clEnqueueAcquireGLObjects::Create(
    cl_command_queue  command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueAcquireGLObjects(
                   command_queue,
                   num_objects,
                   mem_objects,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueAcquireGLObjects;
    m_command_queue = command_queue;
    GetContextInfo();
    m_num_objects = num_objects;
    //m_mem_list = mem_list;
    DeepCopyArray(&m_mem_objects, mem_objects, num_objects);
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueReleaseGLObjects::Create(
    cl_command_queue  command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueReleaseGLObjects(
                   command_queue,
                   num_objects,
                   mem_objects,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueReleaseGLObjects;
    m_command_queue = command_queue;
    GetContextInfo();
    m_num_objects = num_objects;
    //m_mem_list = mem_list;
    DeepCopyArray(&m_mem_objects, mem_objects, num_objects);
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueMarker::Create(
    cl_command_queue  command_queue,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueMarker(command_queue, pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_type = CL_FUNC_TYPE_clEnqueueMarker;
    m_command_queue = command_queue;
    GetContextInfo();

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

void CLAPI_clEnqueueWaitForEvents::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_command_queue  command_queue,
    cl_uint  num_events,
    const cl_event*   event_list,
    cl_int retVal)
{
    m_ullStart = ullStartTime;
    m_ullEnd = ullEndTime;
    m_type = CL_FUNC_TYPE_clEnqueueWaitForEvents;
    m_command_queue = command_queue;
    m_num_events = num_events;

    m_event_wait_list = event_list;
    CopyEventList(event_list, num_events, m_event_list);
    m_retVal = retVal;
}

void CLAPI_clEnqueueBarrier::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_command_queue  command_queue,
    cl_int retVal)
{
    m_ullStart = ullStartTime;
    m_ullEnd = ullEndTime;
    m_type = CL_FUNC_TYPE_clEnqueueBarrier;
    m_command_queue = command_queue;
    m_retVal = retVal;
}

#ifdef _WIN32
cl_int CLAPI_clEnqueueAcquireD3D10ObjectsKHR::Create(
    cl_command_queue  command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = ((clEnqueueAcquireD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[4]))(
                   command_queue,
                   num_objects,
                   mem_objects,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR;
    m_command_queue = command_queue;
    GetContextInfo();
    m_num_objects = num_objects;
    //m_mem_list = mem_list;
    DeepCopyArray(&m_mem_objects, mem_objects, num_objects);
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueReleaseD3D10ObjectsKHR::Create(
    cl_command_queue  command_queue,
    cl_uint num_objects,
    const cl_mem* mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event* event_wait_list,
    cl_event*   event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = ((clEnqueueReleaseD3D10ObjectsKHR_fn)(g_nextDispatchTable._reservedForD3D10KHR[5]))(
                   command_queue,
                   num_objects,
                   mem_objects,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR;
    m_command_queue = command_queue;
    GetContextInfo();
    m_num_objects = num_objects;
    //m_mem_list = mem_list;
    DeepCopyArray(&m_mem_objects, mem_objects, num_objects);
    m_num_events_in_wait_list = num_events_in_wait_list;

    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}
#endif

cl_int CLAPI_clEnqueueFillBuffer::Create(
    cl_command_queue command_queue,
    cl_mem           buffer,
    const void*      pattern,
    size_t           pattern_size,
    size_t           offset,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueFillBuffer(
                   command_queue,
                   buffer,
                   pattern,
                   pattern_size,
                   offset,
                   size,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueFillBuffer;
    m_command_queue = command_queue;
    GetContextInfo();
    m_buffer = buffer;
    m_pattern = pattern;
    m_pattern_size = pattern_size;
    m_offset = offset;
    m_size = size;

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueFillImage::Create(
    cl_command_queue command_queue,
    cl_mem           image,
    const void*      fill_color,
    const size_t     origin[3] ,
    const size_t     region[3] ,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueFillImage(
                   command_queue,
                   image,
                   fill_color,
                   origin,
                   region,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueFillImage;
    m_command_queue = command_queue;
    GetContextInfo();
    m_image = image;
    m_fill_color = fill_color;

    m_origin_null = origin == NULL;

    if (!m_origin_null)
    {
        m_origin[ 0 ] = origin[ 0 ];
        m_origin[ 1 ] = origin[ 1 ];
        m_origin[ 2 ] = origin[ 2 ];
    }

    m_region_null = region == NULL;

    if (!m_region_null)
    {
        m_region[ 0 ] = region[ 0 ];
        m_region[ 1 ] = region[ 1 ];
        m_region[ 2 ] = region[ 2 ];
    }

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    // retrieve image format from runtime so that we can compute data transfer size
    cl_int getImageInfoRet = GetRealDispatchTable()->GetImageInfo(image, CL_IMAGE_FORMAT, sizeof(cl_image_format), &m_format, NULL);

    if (getImageInfoRet != CL_SUCCESS)
    {
        m_format.image_channel_data_type = 0;
        m_format.image_channel_order = 0;
    }


    return m_retVal;
}

cl_int CLAPI_clEnqueueMigrateMemObjects::Create(
    cl_command_queue       command_queue,
    cl_uint                num_mem_objects,
    const cl_mem*          mem_objects,
    cl_mem_migration_flags flags,
    cl_uint                num_events_in_wait_list,
    const cl_event*        event_wait_list,
    cl_event*              event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueMigrateMemObjects(
                   command_queue,
                   num_mem_objects,
                   mem_objects,
                   flags,
                   num_events_in_wait_list,
                   event_wait_list,
                   pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);

    m_type = CL_FUNC_TYPE_clEnqueueMigrateMemObjects;
    m_command_queue = command_queue;
    GetContextInfo();

    m_num_mem_objects = num_mem_objects;
    DeepCopyArray(&m_mem_objects, mem_objects, num_mem_objects);

    m_flags = flags;

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueMarkerWithWaitList::Create(
    cl_command_queue command_queue,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueMarkerWithWaitList(command_queue,
                                                             num_events_in_wait_list,
                                                             event_wait_list,
                                                             pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_type = CL_FUNC_TYPE_clEnqueueMarkerWithWaitList;
    m_command_queue = command_queue;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueBarrierWithWaitList::Create(
    cl_command_queue command_queue,
    cl_uint          num_events_in_wait_list,
    const cl_event*  event_wait_list,
    cl_event*        event)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    m_retVal = g_nextDispatchTable.EnqueueBarrierWithWaitList(command_queue,
                                                              num_events_in_wait_list,
                                                              event_wait_list,
                                                              pTmpEvent);

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_type = CL_FUNC_TYPE_clEnqueueBarrierWithWaitList;
    m_command_queue = command_queue;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueSVMFree::Create(cl_command_queue command_queue,
                                      cl_uint          num_svm_pointers,
                                      void*            svm_pointers[],
                                      void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
                                      void*            user_data,
                                      cl_uint          num_events_in_wait_list,
                                      const cl_event*  event_wait_list,
                                      cl_event*        event,
                                      bool             isExtension)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    if (isExtension)
    {
        m_retVal = g_realExtensionFunctionTable.EnqueueSVMFreeAMD(command_queue,
                                                                  num_svm_pointers,
                                                                  svm_pointers,
                                                                  pfn_free_func,
                                                                  user_data,
                                                                  num_events_in_wait_list,
                                                                  event_wait_list,
                                                                  pTmpEvent);

        m_type = CL_FUNC_TYPE_clEnqueueSVMFreeAMD;
    }
    else
    {
        m_retVal = g_nextDispatchTable.EnqueueSVMFree(command_queue,
                                                      num_svm_pointers,
                                                      svm_pointers,
                                                      pfn_free_func,
                                                      user_data,
                                                      num_events_in_wait_list,
                                                      event_wait_list,
                                                      pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMFree;
    }

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_command_queue = command_queue;
    m_num_svm_pointers = num_svm_pointers;

    for (cl_uint i = 0; i < num_svm_pointers; i++)
    {
        m_pointer_list.push_back(svm_pointers[i]);
    }

    m_pfn_free_func = pfn_free_func;
    m_user_data = user_data;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueSVMMemcpy::Create(cl_command_queue command_queue,
                                        cl_bool          blocking_copy,
                                        void*            dst_ptr,
                                        const void*      src_ptr,
                                        size_t           size,
                                        cl_uint          num_events_in_wait_list,
                                        const cl_event*  event_wait_list,
                                        cl_event*        event,
                                        bool             isExtension)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    if (isExtension)
    {
        m_retVal = g_realExtensionFunctionTable.EnqueueSVMMemcpyAMD(command_queue,
                                                                    blocking_copy,
                                                                    dst_ptr,
                                                                    src_ptr,
                                                                    size,
                                                                    num_events_in_wait_list,
                                                                    event_wait_list,
                                                                    pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMemcpyAMD;
    }
    else
    {
        m_retVal = g_nextDispatchTable.EnqueueSVMMemcpy(command_queue,
                                                        blocking_copy,
                                                        dst_ptr,
                                                        src_ptr,
                                                        size,
                                                        num_events_in_wait_list,
                                                        event_wait_list,
                                                        pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMemcpy;
    }

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_command_queue = command_queue;
    m_blocking_copy = blocking_copy;
    m_dst_ptr = dst_ptr;
    m_src_ptr = src_ptr;
    m_size = size;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueSVMMemFill::Create(cl_command_queue command_queue,
                                         void*            svm_ptr,
                                         const void*      pattern,
                                         size_t           pattern_size,
                                         size_t           size,
                                         cl_uint          num_events_in_wait_list,
                                         const cl_event*  event_wait_list,
                                         cl_event*        event,
                                         bool             isExtension)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    if (isExtension)
    {
        m_retVal = g_realExtensionFunctionTable.EnqueueSVMMemFillAMD(command_queue,
                                                                     svm_ptr,
                                                                     pattern,
                                                                     pattern_size,
                                                                     size,
                                                                     num_events_in_wait_list,
                                                                     event_wait_list,
                                                                     pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMemFillAMD;
    }
    else
    {
        m_retVal = g_nextDispatchTable.EnqueueSVMMemFill(command_queue,
                                                         svm_ptr,
                                                         pattern,
                                                         pattern_size,
                                                         size,
                                                         num_events_in_wait_list,
                                                         event_wait_list,
                                                         pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMemFill;
    }

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_command_queue = command_queue;
    m_svm_ptr = svm_ptr;
    m_pattern = pattern;
    m_pattern_size = pattern_size;
    m_size = size;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueSVMMap::Create(cl_command_queue command_queue,
                                     cl_bool          blocking_map,
                                     cl_map_flags     flags,
                                     void*            svm_ptr,
                                     size_t           size,
                                     cl_uint          num_events_in_wait_list,
                                     const cl_event*  event_wait_list,
                                     cl_event*        event,
                                     bool             isExtension)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    if (isExtension)
    {
        m_retVal = g_realExtensionFunctionTable.EnqueueSVMMapAMD(command_queue,
                                                                 blocking_map,
                                                                 flags,
                                                                 svm_ptr,
                                                                 size,
                                                                 num_events_in_wait_list,
                                                                 event_wait_list,
                                                                 pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMapAMD;
    }
    else
    {
        m_retVal = g_nextDispatchTable.EnqueueSVMMap(command_queue,
                                                     blocking_map,
                                                     flags,
                                                     svm_ptr,
                                                     size,
                                                     num_events_in_wait_list,
                                                     event_wait_list,
                                                     pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMMap;
    }

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_command_queue = command_queue;
    m_blocking_map = blocking_map;
    m_flags = flags;
    m_svm_ptr = svm_ptr;
    m_size = size;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}

cl_int CLAPI_clEnqueueSVMUnmap::Create(cl_command_queue command_queue,
                                       void*            svm_ptr,
                                       cl_uint          num_events_in_wait_list,
                                       const cl_event*  event_wait_list,
                                       cl_event*        event,
                                       bool             isExtension)
{
    cl_event* pTmpEvent = event;
    cl_event event1;

    bool bUserEvent = true;

    if (pTmpEvent == NULL)
    {
        pTmpEvent = &event1;
        bUserEvent = false;
    }

    m_ullStart = CLAPIInfoManager::Instance()->GetTimeNanosStart(this);

    if (isExtension)
    {
        m_retVal = g_realExtensionFunctionTable.EnqueueSVMUnmapAMD(command_queue,
                                                                   svm_ptr,
                                                                   num_events_in_wait_list,
                                                                   event_wait_list,
                                                                   pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMUnmapAMD;
    }
    else
    {
        m_retVal = g_nextDispatchTable.EnqueueSVMUnmap(command_queue,
                                                       svm_ptr,
                                                       num_events_in_wait_list,
                                                       event_wait_list,
                                                       pTmpEvent);
        m_type = CL_FUNC_TYPE_clEnqueueSVMUnmap;
    }

    m_ullEnd = CLAPIInfoManager::Instance()->GetTimeNanosEnd(this);
    m_command_queue = command_queue;
    m_svm_ptr = svm_ptr;
    GetContextInfo();

    m_num_events_in_wait_list = num_events_in_wait_list;
    m_event_wait_list = event_wait_list;
    CopyEventList(event_wait_list, m_num_events_in_wait_list, m_vecEvent_wait_list);

    if (event != NULL)
    {
        m_event = *event;
    }
    else
    {
        m_event = NULL;
    }

    if (this->GetAPISucceeded())
    {
        m_pEvent = CLEventManager::Instance()->UpdateEvent(*pTmpEvent, bUserEvent, this);
    }

    return m_retVal;
}
