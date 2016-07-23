//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file implement all enqueue cmd create functions
//==============================================================================

#ifndef _CL_ENQUEUE_API_DEFS_H_
#define _CL_ENQUEUE_API_DEFS_H_

/// \ingroup CLAPIDefs
// @{

#include <sstream>
#include <cstdlib>
#include <vector>
#include "CLAPIDefBase.h"
#include "CLStringUtils.h"
#include "CLEventManager.h"
#include "CLTraceAgent.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "../Common/StringUtils.h"

#ifdef CL_TRACE_TEST
    #include "../../Non-OpenSource/Tests/CLAPITraceTest/CLAPITraceTest.h"
    #define FRIENDTESTCASE(classname) friend class CLAPITraceTest::classname##Test
    #define RETVALMIN(x,y) x <= y ? x : y
    #define REPLACEDNULLVAL(x,y) x ? NULL : y
#else
    #define FRIENDTESTCASE(classname)
#endif

static size_t GetImageFormatSizeInByte(cl_image_format format)
{
    size_t nChannel = 0;

    switch (format.image_channel_order)
    {
        case CL_R:
        case CL_A:
        case CL_INTENSITY:
        case CL_LUMINANCE:
            nChannel = 1;
            break;

        case CL_RG:
        case CL_RA:
            nChannel = 2;
            break;

        case CL_RGB:
            nChannel = 3;
            break;

        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
            nChannel = 4;
            break;

        default:
            return 0;
    }

    size_t sizeInByte = 0;

    switch (format.image_channel_data_type)
    {
        // 8 bit
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            sizeInByte = 1;
            break;

        // 16 bit
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
        case CL_HALF_FLOAT:
        case CL_UNORM_SHORT_555:
        case CL_UNORM_SHORT_565:
            sizeInByte = 2;
            break;

        // 32 bit
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
        case CL_UNORM_INT_101010:
            sizeInByte = 4;
            break;

        default:
            return 0;
    }

    return sizeInByte * nChannel;
}


//------------------------------------------------------------------------------------
/// clEnqueueReadBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueReadBuffer : public CLEnqueueDataTransfer
{
    FRIENDTESTCASE(clEnqueueReadBuffer);
public:
    /// Constructor
    CLAPI_clEnqueueReadBuffer()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueReadBuffer()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_read) << s_strParamSeparator
           << m_offset << s_strParamSeparator
           << m_cb << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_cb;
    }

    /// Save the parameter values, return value and time stamps of clEnqueueReadBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueReadBuffer
    /// \param buffer Parameter for CLAPI_clEnqueueReadBuffer
    /// \param blocking_read Parameter for CLAPI_clEnqueueReadBuffer
    /// \param offset Parameter for CLAPI_clEnqueueReadBuffer
    /// \param cb Parameter for CLAPI_clEnqueueReadBuffer
    /// \param ptr Parameter for CLAPI_clEnqueueReadBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueReadBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueReadBuffer
    /// \param event Parameter for CLAPI_clEnqueueReadBuffer
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   buffer,
        cl_bool  blocking_read,
        size_t   offset,
        size_t   cb,
        void* ptr,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueReadBuffer(const CLAPI_clEnqueueReadBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueReadBuffer& operator = (const CLAPI_clEnqueueReadBuffer& obj);

private:
    cl_mem   m_buffer;        ///< parameter for clEnqueueReadBuffer
    cl_bool  m_blocking_read; ///< parameter for clEnqueueReadBuffer
    size_t   m_offset;        ///< parameter for clEnqueueReadBuffer
    size_t   m_cb;            ///< parameter for clEnqueueReadBuffer
    void*    m_ptr;           ///< parameter for clEnqueueReadBuffer
    cl_event m_event;         ///< parameter for clEnqueueReadBuffer
    cl_int   m_retVal;        ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueReadBufferRect
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueReadBufferRect : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueReadBufferRect()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueReadBufferRect()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2];
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_read) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_buffer_offset_null, m_buffer_offset), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_host_offset_null, m_host_offset), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_buffer_row_pitch << s_strParamSeparator
           << m_buffer_slice_pitch << s_strParamSeparator
           << m_host_row_pitch << s_strParamSeparator
           << m_host_slice_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueReadBufferRect
    /// \param command_queue Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param buffer Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param blocking_read Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param buffer_offset Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param host_offset Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param region Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param buffer_row_pitch Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param buffer_slice_pitch Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param host_row_pitch Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param host_slice_pitch Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param ptr Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param event_wait_list Parameter for CLAPI_clEnqueueReadBufferRect
    /// \param event Parameter for CLAPI_clEnqueueReadBufferRect
    cl_int Create(
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
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueReadBufferRect(const CLAPI_clEnqueueReadBufferRect& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueReadBufferRect& operator = (const CLAPI_clEnqueueReadBufferRect& obj);

private:
    cl_mem   m_buffer;             ///< parameter for clEnqueueReadBufferRect
    cl_bool  m_blocking_read;      ///< parameter for clEnqueueReadBufferRect
    size_t   m_buffer_offset[3];   ///< parameter for clEnqueueReadBufferRect
    size_t   m_host_offset[3];     ///< parameter for clEnqueueReadBufferRect
    size_t   m_region[3];          ///< parameter for clEnqueueReadBufferRect
    size_t   m_buffer_row_pitch;   ///< parameter for clEnqueueReadBufferRect
    size_t   m_buffer_slice_pitch; ///< parameter for clEnqueueReadBufferRect
    size_t   m_host_row_pitch;     ///< parameter for clEnqueueReadBufferRect
    size_t   m_host_slice_pitch;   ///< parameter for clEnqueueReadBufferRect
    void*    m_ptr;                ///< parameter for clEnqueueReadBufferRect
    cl_event m_event;              ///< parameter for clEnqueueReadBufferRect
    cl_int   m_retVal;             ///< return value
    bool     m_buffer_offset_null; ///< flag indicating that m_buffer_offset is null;
    bool     m_host_offset_null;   ///< flag indicating that m_host_offset is null;
    bool     m_region_null;        ///< flag indicating that m_region is null;
};

//------------------------------------------------------------------------------------
/// clEnqueueWriteBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueWriteBuffer : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueWriteBuffer()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueWriteBuffer()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_cb;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_write) << s_strParamSeparator
           << m_offset << s_strParamSeparator
           << m_cb << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueWriteBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param buffer Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param blocking_write Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param offset Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param cb Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param ptr Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueWriteBuffer
    /// \param event Parameter for CLAPI_clEnqueueWriteBuffer
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   buffer,
        cl_bool  blocking_write,
        size_t   offset,
        size_t   cb,
        const void* ptr,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueWriteBuffer(const CLAPI_clEnqueueWriteBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueWriteBuffer& operator = (const CLAPI_clEnqueueWriteBuffer& obj);

private:
    cl_mem      m_buffer;         ///< parameter for clEnqueueWriteBuffer
    cl_bool     m_blocking_write; ///< parameter for clEnqueueWriteBuffer
    size_t      m_offset;         ///< parameter for clEnqueueWriteBuffer
    size_t      m_cb;             ///< parameter for clEnqueueWriteBuffer
    const void* m_ptr;            ///< parameter for clEnqueueWriteBuffer
    cl_event    m_event;          ///< parameter for clEnqueueWriteBuffer
    cl_int      m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueWriteBufferRect
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueWriteBufferRect : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueWriteBufferRect()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueWriteBufferRect()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2];
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_read) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_buffer_offset_null, m_buffer_offset), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_host_offset_null, m_host_offset), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_buffer_row_pitch << s_strParamSeparator
           << m_buffer_slice_pitch << s_strParamSeparator
           << m_host_row_pitch << s_strParamSeparator
           << m_host_slice_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueWriteBufferRect
    /// \param command_queue Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param buffer Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param blocking_read Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param buffer_offset Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param host_offset Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param region Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param buffer_row_pitch Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param buffer_slice_pitch Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param host_row_pitch Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param host_slice_pitch Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param ptr Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param event_wait_list Parameter for CLAPI_clEnqueueWriteBufferRect
    /// \param event Parameter for CLAPI_clEnqueueWriteBufferRect
    cl_int Create(
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
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueWriteBufferRect(const CLAPI_clEnqueueWriteBufferRect& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueWriteBufferRect& operator = (const CLAPI_clEnqueueWriteBufferRect& obj);

private:
    cl_mem      m_buffer;             ///< parameter for clEnqueueWriteBufferRect
    cl_bool     m_blocking_read;      ///< parameter for clEnqueueWriteBufferRect
    size_t      m_buffer_offset[3];   ///< parameter for clEnqueueWriteBufferRect
    size_t      m_host_offset[3];     ///< parameter for clEnqueueWriteBufferRect
    size_t      m_region[3];          ///< parameter for clEnqueueWriteBufferRect
    size_t      m_buffer_row_pitch;   ///< parameter for clEnqueueWriteBufferRect
    size_t      m_buffer_slice_pitch; ///< parameter for clEnqueueWriteBufferRect
    size_t      m_host_row_pitch;     ///< parameter for clEnqueueWriteBufferRect
    size_t      m_host_slice_pitch;   ///< parameter for clEnqueueWriteBufferRect
    const void* m_ptr;                ///< parameter for clEnqueueWriteBufferRect
    cl_event    m_event;              ///< parameter for clEnqueueWriteBufferRect
    cl_int      m_retVal;             ///< return value
    bool        m_buffer_offset_null; ///< flag indicating that m_buffer_offset is null;
    bool        m_host_offset_null;   ///< flag indicating that m_host_offset is null;
    bool        m_region_null;        ///< flag indicating that m_region is null;
};

//------------------------------------------------------------------------------------
/// clEnqueueCopyBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueCopyBuffer : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueCopyBuffer()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueCopyBuffer()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_cb;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_buffer) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_buffer) << s_strParamSeparator
           << m_src_offset << s_strParamSeparator
           << m_dst_offset << s_strParamSeparator
           << m_cb << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueCopyBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param src_buffer Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param dst_buffer Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param src_offset Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param dst_offset Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param cb Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueCopyBuffer
    /// \param event Parameter for CLAPI_clEnqueueCopyBuffer
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   src_buffer,
        cl_mem   dst_buffer,
        size_t   src_offset,
        size_t   dst_offset,
        size_t   cb,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueCopyBuffer(const CLAPI_clEnqueueCopyBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueCopyBuffer& operator = (const CLAPI_clEnqueueCopyBuffer& obj);

private:
    cl_mem   m_src_buffer; ///< parameter for clEnqueueCopyBuffer
    cl_mem   m_dst_buffer; ///< parameter for clEnqueueCopyBuffer
    size_t   m_src_offset; ///< parameter for clEnqueueCopyBuffer
    size_t   m_dst_offset; ///< parameter for clEnqueueCopyBuffer
    size_t   m_cb;         ///< parameter for clEnqueueCopyBuffer
    cl_event m_event;      ///< parameter for clEnqueueCopyBuffer
    cl_int m_retVal;       ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueCopyBufferRect
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueCopyBufferRect : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueCopyBufferRect()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueCopyBufferRect()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2];
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_buffer) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_buffer) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_src_origin_null, m_src_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_dst_origin_null, m_dst_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_src_row_pitch << s_strParamSeparator
           << m_src_slice_pitch << s_strParamSeparator
           << m_dst_row_pitch << s_strParamSeparator
           << m_dst_slice_pitch << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueCopyBufferRect
    /// \param command_queue Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param src_buffer Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param dst_buffer Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param src_origin Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param dst_origin Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param region Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param src_row_pitch Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param src_slice_pitch Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param dst_row_pitch Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param dst_slice_pitch Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param event_wait_list Parameter for CLAPI_clEnqueueCopyBufferRect
    /// \param event Parameter for CLAPI_clEnqueueCopyBufferRect
    cl_int Create(
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
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueCopyBufferRect(const CLAPI_clEnqueueCopyBufferRect& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueCopyBufferRect& operator = (const CLAPI_clEnqueueCopyBufferRect& obj);

private:
    cl_mem   m_src_buffer;      ///< parameter for clEnqueueCopyBufferRect
    cl_mem   m_dst_buffer;      ///< parameter for clEnqueueCopyBufferRect
    size_t   m_src_origin[3];   ///< parameter for clEnqueueCopyBufferRect
    size_t   m_dst_origin[3];   ///< parameter for clEnqueueCopyBufferRect
    size_t   m_region[3];       ///< parameter for clEnqueueCopyBufferRect
    size_t   m_src_row_pitch;   ///< parameter for clEnqueueCopyBufferRect
    size_t   m_src_slice_pitch; ///< parameter for clEnqueueCopyBufferRect
    size_t   m_dst_row_pitch;   ///< parameter for clEnqueueCopyBufferRect
    size_t   m_dst_slice_pitch; ///< parameter for clEnqueueCopyBufferRect
    cl_event m_event;           ///< parameter for clEnqueueCopyBufferRect
    cl_int   m_retVal;          ///< return value
    bool     m_src_origin_null; ///< flag indicating that m_src_origin was NULL
    bool     m_dst_origin_null; ///< flag indicating that m_dst_origin was NULL
    bool     m_region_null;     ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueReadImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueReadImage : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueReadImage()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueReadImage()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_image) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_read) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_origin_null, m_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_row_pitch << s_strParamSeparator
           << m_slice_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueReadImage
    /// \param command_queue Parameter for CLAPI_clEnqueueReadImage
    /// \param image Parameter for CLAPI_clEnqueueReadImage
    /// \param blocking_read Parameter for CLAPI_clEnqueueReadImage
    /// \param origin Parameter for CLAPI_clEnqueueReadImage
    /// \param region Parameter for CLAPI_clEnqueueReadImage
    /// \param row_pitch Parameter for CLAPI_clEnqueueReadImage
    /// \param slice_pitch Parameter for CLAPI_clEnqueueReadImage
    /// \param ptr Parameter for CLAPI_clEnqueueReadImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueReadImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueReadImage
    /// \param event Parameter for CLAPI_clEnqueueReadImage
    cl_int Create(
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
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueReadImage(const CLAPI_clEnqueueReadImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueReadImage& operator = (const CLAPI_clEnqueueReadImage& obj);

private:
    cl_mem          m_image;         ///< parameter for clEnqueueReadImage
    cl_bool         m_blocking_read; ///< parameter for clEnqueueReadImage
    size_t          m_origin[3];     ///< parameter for clEnqueueReadImage
    size_t          m_region[3];     ///< parameter for clEnqueueReadImage
    size_t          m_row_pitch;     ///< parameter for clEnqueueReadImage
    size_t          m_slice_pitch;   ///< parameter for clEnqueueReadImage
    void*           m_ptr;           ///< parameter for clEnqueueReadImage
    cl_event        m_event;         ///< parameter for clEnqueueReadImage
    cl_int          m_retVal;        ///< return value
    cl_image_format m_format;        ///< image format
    bool            m_origin_null;   ///< flag indicating that m_origin was NULL
    bool            m_region_null;   ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueWriteImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueWriteImage : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueWriteImage()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueWriteImage()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_image) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_write) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_origin_null, m_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_input_row_pitch << s_strParamSeparator
           << m_input_slice_pitch << s_strParamSeparator
           << StringUtils::ToHexString(m_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueWriteImage
    /// \param command_queue Parameter for CLAPI_clEnqueueWriteImage
    /// \param image Parameter for CLAPI_clEnqueueWriteImage
    /// \param blocking_write Parameter for CLAPI_clEnqueueWriteImage
    /// \param origin Parameter for CLAPI_clEnqueueWriteImage
    /// \param region Parameter for CLAPI_clEnqueueWriteImage
    /// \param input_row_pitch Parameter for CLAPI_clEnqueueWriteImage
    /// \param input_slice_pitch Parameter for CLAPI_clEnqueueWriteImage
    /// \param ptr Parameter for CLAPI_clEnqueueWriteImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueWriteImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueWriteImage
    /// \param event Parameter for CLAPI_clEnqueueWriteImage
    cl_int Create(
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
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueWriteImage(const CLAPI_clEnqueueWriteImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueWriteImage& operator = (const CLAPI_clEnqueueWriteImage& obj);

private:
    cl_mem          m_image;             ///< parameter for clEnqueueWriteImage
    cl_bool         m_blocking_write;    ///< parameter for clEnqueueWriteImage
    size_t          m_origin[3];         ///< parameter for clEnqueueWriteImage
    size_t          m_region[3];         ///< parameter for clEnqueueWriteImage
    size_t          m_input_row_pitch;   ///< parameter for clEnqueueWriteImage
    size_t          m_input_slice_pitch; ///< parameter for clEnqueueWriteImage
    const void*     m_ptr;               ///< parameter for clEnqueueWriteImage
    cl_event        m_event;             ///< parameter for clEnqueueWriteImage
    cl_int          m_retVal;            ///< return value
    cl_image_format m_format;            ///< image format
    bool            m_origin_null;       ///< flag indicating that m_origin was NULL
    bool            m_region_null;       ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueCopyImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueCopyImage : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueCopyImage()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueCopyImage()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_image) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_image) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_src_origin_null, m_src_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_dst_origin_null, m_dst_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueCopyImage
    /// \param command_queue Parameter for CLAPI_clEnqueueCopyImage
    /// \param src_image Parameter for CLAPI_clEnqueueCopyImage
    /// \param dst_image Parameter for CLAPI_clEnqueueCopyImage
    /// \param src_origin Parameter for CLAPI_clEnqueueCopyImage
    /// \param dst_origin Parameter for CLAPI_clEnqueueCopyImage
    /// \param region Parameter for CLAPI_clEnqueueCopyImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueCopyImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueCopyImage
    /// \param event Parameter for CLAPI_clEnqueueCopyImage
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   src_image,
        cl_mem   dst_image,
        const size_t   src_origin[3],
        const size_t   dst_origin[3],
        const size_t   region[3],
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueCopyImage(const CLAPI_clEnqueueCopyImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueCopyImage& operator = (const CLAPI_clEnqueueCopyImage& obj);

private:
    cl_mem          m_src_image;       ///< parameter for clEnqueueCopyImage
    cl_mem          m_dst_image;       ///< parameter for clEnqueueCopyImage
    size_t          m_src_origin[3];   ///< parameter for clEnqueueCopyImage
    size_t          m_dst_origin[3];   ///< parameter for clEnqueueCopyImage
    size_t          m_region[3];       ///< parameter for clEnqueueCopyImage
    cl_event        m_event;           ///< parameter for clEnqueueCopyImage
    cl_int          m_retVal;          ///< return value
    cl_image_format m_format;          ///< image format
    bool            m_src_origin_null; ///< flag indicating that m_src_origin was NULL
    bool            m_dst_origin_null; ///< flag indicating that m_dst_origin was NULL
    bool            m_region_null;     ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueCopyImageToBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueCopyImageToBuffer : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueCopyImageToBuffer()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueCopyImageToBuffer()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_image) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_buffer) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_src_origin_null, m_src_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_dst_offset << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueCopyImageToBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param src_image Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param dst_buffer Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param src_origin Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param region Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param dst_offset Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueCopyImageToBuffer
    /// \param event Parameter for CLAPI_clEnqueueCopyImageToBuffer
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   src_image,
        cl_mem   dst_buffer,
        const size_t   src_origin[3],
        const size_t   region[3],
        size_t   dst_offset,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueCopyImageToBuffer(const CLAPI_clEnqueueCopyImageToBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueCopyImageToBuffer& operator = (const CLAPI_clEnqueueCopyImageToBuffer& obj);

private:
    cl_mem          m_src_image;       ///< parameter for clEnqueueCopyImageToBuffer
    cl_mem          m_dst_buffer;      ///< parameter for clEnqueueCopyImageToBuffer
    size_t          m_src_origin[3];   ///< parameter for clEnqueueCopyImageToBuffer
    size_t          m_region[3];       ///< parameter for clEnqueueCopyImageToBuffer
    size_t          m_dst_offset;      ///< parameter for clEnqueueCopyImageToBuffer
    cl_event        m_event;           ///< parameter for clEnqueueCopyImageToBuffer
    cl_int          m_retVal;          ///< return value
    cl_image_format m_format;          ///< image format
    bool            m_src_origin_null; ///< flag indicating that m_src_origin was NULL
    bool            m_region_null;     ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueCopyBufferToImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueCopyBufferToImage : public CLEnqueueDataTransfer
{
public:
    /// Constructor
    CLAPI_clEnqueueCopyBufferToImage()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueCopyBufferToImage()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_buffer) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_image) << s_strParamSeparator
           << m_src_offset << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_dst_origin_null, m_dst_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueCopyBufferToImage
    /// \param command_queue Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param src_buffer Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param dst_image Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param src_offset Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param dst_origin Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param region Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueCopyBufferToImage
    /// \param event Parameter for CLAPI_clEnqueueCopyBufferToImage
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   src_buffer,
        cl_mem   dst_image,
        size_t   src_offset,
        const size_t   dst_origin[3],
        const size_t   region[3],
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueCopyBufferToImage(const CLAPI_clEnqueueCopyBufferToImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueCopyBufferToImage& operator = (const CLAPI_clEnqueueCopyBufferToImage& obj);

private:
    cl_mem          m_src_buffer;      ///< parameter for clEnqueueCopyBufferToImage
    cl_mem          m_dst_image;       ///< parameter for clEnqueueCopyBufferToImage
    size_t          m_src_offset;      ///< parameter for clEnqueueCopyBufferToImage
    size_t          m_dst_origin[3];   ///< parameter for clEnqueueCopyBufferToImage
    size_t          m_region[3];       ///< parameter for clEnqueueCopyBufferToImage
    cl_event        m_event;           ///< parameter for clEnqueueCopyBufferToImage
    cl_int          m_retVal;          ///< return value
    cl_image_format m_format;          ///< image format
    bool            m_dst_origin_null; ///< flag indicating that m_dst_origin was NULL
    bool            m_region_null;     ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueMapMemObject base class
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMapMemObj : public CLEnqueueDataTransfer
{
public:
    //------------------------------------------------------------------------------------
    /// Device type
    //------------------------------------------------------------------------------------
    enum DeviceType
    {
        DT_Unknown,       ///< Unknown device
        DT_DiscreteGPU,   ///< Discrete GPU
        DT_CPU,           ///< CPU
        DT_APU,           ///< APU
        DT_Last           ///< Last
    };

    //------------------------------------------------------------------------------------
    /// Memory location
    //------------------------------------------------------------------------------------
    enum MemLocation
    {
        ML_Unknown,                ///< Unknown
        ML_HostMem,                ///< Host memory
        ML_PinnedHostMem,          ///< Pinned host memory
        ML_DeviceMem,              ///< Device memory
        ML_DeviceVisibleHostMem,   ///< Device visible host memory
        ML_HostVisibleDeviceMem,   ///< Host visible device memory
        ML_Last                    ///< Last
    };

    /// Constructor
    CLAPI_clEnqueueMapMemObj()
    {
        m_deviceType = DT_Unknown;
        m_bCPUOnlyContext = false;
    }

    /// Destructor
    virtual ~CLAPI_clEnqueueMapMemObj()
    {}

    /// Helper function
    /// \param memLoc Memory location
    /// \return memory location string
    static std::string ToString(MemLocation memLoc)
    {
        switch (memLoc)
        {
            case ML_HostMem:
                return std::string("Host memory");

            case ML_PinnedHostMem:
                return std::string("Pinned host memory");

            case ML_DeviceMem:
                return std::string("Device memory");

            case ML_DeviceVisibleHostMem:
                return std::string("Device visible host memory");

            case ML_HostVisibleDeviceMem:
                return std::string("Host visible device memory");

            default:
                return std::string("Unknown memory location");
        }
    }

    /// Helper function
    /// \param type Device type
    /// \return device type string
    static std::string ToString(DeviceType type)
    {
        switch (type)
        {
            case DT_APU:
                return std::string("APU");

            case DT_CPU:
                return std::string("CPU");

            case DT_DiscreteGPU:
                return std::string("Discrete GPU");

            default:
                return std::string("Unknown device type");
        }
    }

protected:
    /// Get memory object allocation location, map location and zero copy flag from device type and memory flag
    /// \param[out] allocLocation Allocation location
    /// \param[out] mapLocation map location
    /// \param[out] bZeroCopy a flag indicating whether or not map is zero copy
    /// \param[in] bImage Is input memory object an image type
    /// \return true if succeeded
    bool GetLocation(MemLocation& allocLocation, MemLocation& mapLocation, bool& bZeroCopy, bool bImage = false) const;

    /// Helper function to retrieve memory flag and device type from cl_mem object and command queue object
    /// \param buffer cl mem object
    /// \return true if succeeded
    bool GetMemDeviceInfo(cl_mem buffer);

    /// Helper function to get map info string
    /// \param ss stringstream object
    /// \param[in] bImage Is input memory object an image type
    void GetMapInfoString(std::ostringstream& ss, bool bImage = false)
    {
        if (m_deviceType != DT_Unknown)
        {
            MemLocation allocLoc;
            MemLocation mapLoc;
            bool bZeroCopy;

            if (GetLocation(allocLoc, mapLoc, bZeroCopy, bImage))
            {
                ss << " /* ";
                ss << "Device type = " << CLAPI_clEnqueueMapMemObj::ToString(m_deviceType);
                ss << ";Buffer location = " << CLAPI_clEnqueueMapMemObj::ToString(allocLoc);
                ss << ";Map location = " << CLAPI_clEnqueueMapMemObj::ToString(mapLoc);
                ss << ";Zero copy = " << (bZeroCopy ? "True" : "False");
                ss << " */";
            }
        }
    }

    cl_mem_flags m_memFlags;         ///< Memory object flag
    DeviceType   m_deviceType;       ///< device type
    bool         m_bCPUOnlyContext;  ///< Context only has CPU device
};

//------------------------------------------------------------------------------------
/// clEnqueueMapBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMapBuffer : public CLAPI_clEnqueueMapMemObj
{
public:
    /// Constructor
    CLAPI_clEnqueueMapBuffer()
        : CLAPI_clEnqueueMapMemObj()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueMapBuffer()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal != NULL;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_cb;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_map) << s_strParamSeparator
           << CLStringUtils::GetMapFlagsString(m_map_flags) << s_strParamSeparator
           << m_offset << s_strParamSeparator
           << m_cb << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);

        GetMapInfoString(ss);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueMapBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueMapBuffer
    /// \param buffer Parameter for CLAPI_clEnqueueMapBuffer
    /// \param blocking_map Parameter for CLAPI_clEnqueueMapBuffer
    /// \param map_flags Parameter for CLAPI_clEnqueueMapBuffer
    /// \param offset Parameter for CLAPI_clEnqueueMapBuffer
    /// \param cb Parameter for CLAPI_clEnqueueMapBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueMapBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueMapBuffer
    /// \param event Parameter for CLAPI_clEnqueueMapBuffer
    /// \param errcode_ret Parameter for CLAPI_clEnqueueMapBuffer
    void* Create(
        cl_command_queue  command_queue,
        cl_mem   buffer,
        cl_bool  blocking_map,
        cl_map_flags   map_flags,
        size_t   offset,
        size_t   cb,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event,
        cl_int*  errcode_ret);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueMapBuffer(const CLAPI_clEnqueueMapBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueMapBuffer& operator = (const CLAPI_clEnqueueMapBuffer& obj);

private:
    cl_mem       m_buffer;         ///< parameter for clEnqueueMapBuffer
    cl_bool      m_blocking_map;   ///< parameter for clEnqueueMapBuffer
    cl_map_flags m_map_flags;      ///< parameter for clEnqueueMapBuffer
    size_t       m_offset;         ///< parameter for clEnqueueMapBuffer
    size_t       m_cb;             ///< parameter for clEnqueueMapBuffer
    cl_event     m_event;          ///< parameter for clEnqueueMapBuffer
    cl_int*      m_errcode_ret;    ///< parameter for clEnqueueMapBuffer
    cl_int       m_errcode_retVal; ///< parameter for clEnqueueMapBuffer
    void*        m_retVal;         ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueMapImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMapImage : public CLAPI_clEnqueueMapMemObj
{
public:
    /// Constructor
    CLAPI_clEnqueueMapImage()
        : CLAPI_clEnqueueMapMemObj()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueMapImage()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal != NULL;
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_image) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_map) << s_strParamSeparator
           << CLStringUtils::GetMapFlagsString(m_map_flags) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_origin_null, m_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << CLStringUtils::GetSizeString(m_image_row_pitch, m_image_row_pitchVal) << s_strParamSeparator
           << CLStringUtils::GetSizeString(m_image_slice_pitch, m_image_slice_pitchVal) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event) << s_strParamSeparator
           << CLStringUtils::GetErrorString(m_errcode_ret, m_errcode_retVal);

        GetMapInfoString(ss, true);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueMapImage
    /// \param command_queue Parameter for CLAPI_clEnqueueMapImage
    /// \param image Parameter for CLAPI_clEnqueueMapImage
    /// \param blocking_map Parameter for CLAPI_clEnqueueMapImage
    /// \param map_flags Parameter for CLAPI_clEnqueueMapImage
    /// \param origin Parameter for CLAPI_clEnqueueMapImage
    /// \param region Parameter for CLAPI_clEnqueueMapImage
    /// \param image_row_pitch Parameter for CLAPI_clEnqueueMapImage
    /// \param image_slice_pitch Parameter for CLAPI_clEnqueueMapImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueMapImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueMapImage
    /// \param event Parameter for CLAPI_clEnqueueMapImage
    /// \param errcode_ret Parameter for CLAPI_clEnqueueMapImage
    void* Create(
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
        cl_int*  errcode_ret);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueMapImage(const CLAPI_clEnqueueMapImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueMapImage& operator = (const CLAPI_clEnqueueMapImage& obj);

private:
    cl_mem          m_image;                ///< parameter for clEnqueueMapImage
    cl_bool         m_blocking_map;         ///< parameter for clEnqueueMapImage
    cl_map_flags    m_map_flags;            ///< parameter for clEnqueueMapImage
    size_t          m_origin[3];            ///< parameter for clEnqueueMapImage
    size_t          m_region[3];            ///< parameter for clEnqueueMapImage
    size_t*         m_image_row_pitch;      ///< parameter for clEnqueueMapImage
    size_t          m_image_row_pitchVal;   ///< parameter for clEnqueueMapImage
    size_t*         m_image_slice_pitch;    ///< parameter for clEnqueueMapImage
    size_t          m_image_slice_pitchVal; ///< parameter for clEnqueueMapImage
    cl_event        m_event;                ///< parameter for clEnqueueMapImage
    cl_int*         m_errcode_ret;          ///< parameter for clEnqueueMapImage
    cl_int          m_errcode_retVal;       ///< parameter for clEnqueueMapImage
    void*           m_retVal;               ///< return value
    cl_image_format m_format;               ///< image format
    bool            m_origin_null;          ///< flag indicating that m_origin was NULL
    bool            m_region_null;          ///< flag indicating that m_region was NULL
};

//------------------------------------------------------------------------------------
/// clEnqueueUnmapMemObject
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueUnmapMemObject : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueUnmapMemObject()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueUnmapMemObject()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_memobj) << s_strParamSeparator
           << StringUtils::ToHexString(m_mapped_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueUnmapMemObject
    /// \param command_queue Parameter for CLAPI_clEnqueueUnmapMemObject
    /// \param memobj Parameter for CLAPI_clEnqueueUnmapMemObject
    /// \param mapped_ptr Parameter for CLAPI_clEnqueueUnmapMemObject
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueUnmapMemObject
    /// \param event_wait_list Parameter for CLAPI_clEnqueueUnmapMemObject
    /// \param event Parameter for CLAPI_clEnqueueUnmapMemObject
    cl_int Create(
        cl_command_queue  command_queue,
        cl_mem   memobj,
        void* mapped_ptr,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return 0;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueUnmapMemObject(const CLAPI_clEnqueueUnmapMemObject& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueUnmapMemObject& operator = (const CLAPI_clEnqueueUnmapMemObject& obj);

private:
    cl_mem   m_memobj;     ///< parameter for clEnqueueUnmapMemObject
    void*    m_mapped_ptr; ///< parameter for clEnqueueUnmapMemObject
    cl_event m_event;      ///< parameter for clEnqueueUnmapMemObject
    cl_int   m_retVal;     ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueNDRangeKernel
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueNDRangeKernel : public CLEnqueueAPIBase
{
public:
    /// Constructor
    CLAPI_clEnqueueNDRangeKernel()
    {
        m_global_work_offset = NULL;
        m_global_work_size = NULL;
        m_local_work_size = NULL;
    }

    /// Destructor
    ~CLAPI_clEnqueueNDRangeKernel()
    {
        if (m_global_work_offset != NULL)
        {
            FreeArray(m_global_work_offset);
        }

        if (m_global_work_size != NULL)
        {
            FreeArray(m_global_work_size);
        }

        if (m_local_work_size != NULL)
        {
            FreeArray(m_local_work_size);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << m_work_dim << s_strParamSeparator
           << CLStringUtils::GetNDimString(m_global_work_offset, m_work_dim) << s_strParamSeparator
           << CLStringUtils::GetNDimString(m_global_work_size, m_work_dim) << s_strParamSeparator
           << CLStringUtils::GetNDimString(m_local_work_size, m_work_dim) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueNDRangeKernel
    /// \param command_queue Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param kernel Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param work_dim Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param global_work_offset Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param global_work_size Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param local_work_size Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param event_wait_list Parameter for CLAPI_clEnqueueNDRangeKernel
    /// \param event Parameter for CLAPI_clEnqueueNDRangeKernel
    cl_int Create(
        cl_command_queue  command_queue,
        cl_kernel   kernel,
        cl_uint  work_dim,
        const size_t*  global_work_offset,
        const size_t*  global_work_size,
        const size_t*  local_work_size,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

    cl_kernel GetKernel() const
    {
        return m_kernel;
    }

    cl_uint GetWorkDim()
    {
        return m_work_dim;
    }

    size_t* GetGlobalWorkSize() const
    {
        return m_global_work_size;
    }

    size_t* GetLocalWorkSize() const
    {
        return m_local_work_size;
    }

    std::string GetKernelName() const
    {
        return m_strKernelName;
    }

    bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueNDRangeKernel(const CLAPI_clEnqueueNDRangeKernel& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueNDRangeKernel& operator = (const CLAPI_clEnqueueNDRangeKernel& obj);

private:
    cl_kernel   m_kernel;                        ///< parameter for clEnqueueNDRangeKernel
    cl_uint     m_work_dim;                      ///< parameter for clEnqueueNDRangeKernel
    size_t*     m_global_work_offset;            ///< parameter for clEnqueueNDRangeKernel
    size_t*     m_global_work_size;              ///< parameter for clEnqueueNDRangeKernel
    size_t*     m_local_work_size;               ///< parameter for clEnqueueNDRangeKernel
    cl_event    m_event;                         ///< parameter for clEnqueueNDRangeKernel
    cl_int      m_retVal;                        ///< return value
    std::string m_strKernelName;                 ///< kernel name
};

//------------------------------------------------------------------------------------
/// clEnqueueTask
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueTask : public CLEnqueueAPIBase
{
public:
    /// Constructor
    CLAPI_clEnqueueTask()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueTask()
    {

    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_kernel) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueTask
    /// \param command_queue Parameter for CLAPI_clEnqueueTask
    /// \param kernel Parameter for CLAPI_clEnqueueTask
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueTask
    /// \param event_wait_list Parameter for CLAPI_clEnqueueTask
    /// \param event Parameter for CLAPI_clEnqueueTask
    cl_int Create(
        cl_command_queue  command_queue,
        cl_kernel   kernel,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

    cl_kernel GetKernel() const
    {
        return m_kernel;
    }

    bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueTask(const CLAPI_clEnqueueTask& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueTask& operator = (const CLAPI_clEnqueueTask& obj);

private:
    cl_kernel m_kernel; ///< parameter for clEnqueueTask
    cl_event  m_event;  ///< parameter for clEnqueueTask
    cl_int    m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueNativeKernel
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueNativeKernel : public CLEnqueueAPIBase
{
public:
    /// Constructor
    CLAPI_clEnqueueNativeKernel()
    {
        m_mem_list = NULL;
    }

    /// Destructor
    ~CLAPI_clEnqueueNativeKernel()
    {
        if (m_mem_list != NULL)
        {
            FreeArray(m_mem_list);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_func) << s_strParamSeparator
           << StringUtils::ToHexString(m_args) << s_strParamSeparator
           << m_cb_args << s_strParamSeparator
           << m_num_mem_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_list, m_num_mem_objects) << s_strParamSeparator
           << StringUtils::ToHexString(m_args_mem_loc) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueNativeKernel
    /// \param command_queue Parameter for CLAPI_clEnqueueNativeKernel
    /// \param user_func Parameter for CLAPI_clEnqueueNativeKernel
    /// \param args Parameter for CLAPI_clEnqueueNativeKernel
    /// \param cb_args Parameter for CLAPI_clEnqueueNativeKernel
    /// \param num_mem_objects Parameter for CLAPI_clEnqueueNativeKernel
    /// \param mem_list Parameter for CLAPI_clEnqueueNativeKernel
    /// \param args_mem_loc Parameter for CLAPI_clEnqueueNativeKernel
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueNativeKernel
    /// \param event_wait_list Parameter for CLAPI_clEnqueueNativeKernel
    /// \param event Parameter for CLAPI_clEnqueueNativeKernel
    cl_int Create(
        cl_command_queue  command_queue,
        void (CL_CALLBACK* user_func)(void*),
        void* args,
        size_t   cb_args,
        cl_uint  num_mem_objects,
        const cl_mem*  mem_list,
        const void**   args_mem_loc,
        cl_uint  num_events_in_wait_list,
        const cl_event*   event_wait_list,
        cl_event*   event);

    void* GetNativeKernel() const
    {
        return (void*)m_user_func;
    }

    bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueNativeKernel(const CLAPI_clEnqueueNativeKernel& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueNativeKernel& operator = (const CLAPI_clEnqueueNativeKernel& obj);

private:
    void (CL_CALLBACK* m_user_func)(void*); ///< parameter for clEnqueueNativeKernel
    void*        m_args;                    ///< parameter for clEnqueueNativeKernel
    size_t       m_cb_args;                 ///< parameter for clEnqueueNativeKernel
    cl_uint      m_num_mem_objects;         ///< parameter for clEnqueueNativeKernel
    cl_mem*      m_mem_list;                ///< parameter for clEnqueueNativeKernel
    const void** m_args_mem_loc;            ///< parameter for clEnqueueNativeKernel
    cl_event     m_event;                   ///< parameter for clEnqueueNativeKernel
    cl_int       m_retVal;                  ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueMarker
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMarker : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueMarker() {}

    /// Destructor
    ~CLAPI_clEnqueueMarker() {}

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueMarker
    /// \param command_queue Parameter for CLAPI_clEnqueueMarker
    /// \param event Parameter for CLAPI_clEnqueueMarker
    cl_int Create(
        cl_command_queue  command_queue,
        cl_event*   event);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return 0;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueMarker(const CLAPI_clEnqueueMarker& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueMarker& operator = (const CLAPI_clEnqueueMarker& obj);

private:
    cl_event m_event;  ///< parameter for clEnqueueMarker
    cl_int   m_retVal; ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueAcquireGLObjects
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueAcquireGLObjects : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueAcquireGLObjects() {}

    /// Destructor
    ~CLAPI_clEnqueueAcquireGLObjects()
    {
        if (m_mem_objects != NULL)
        {
            FreeArray(m_mem_objects);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_objects, m_num_objects) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueAcquireGLObjects
    /// \param command_queue Parameter for CLAPI_clEnqueueAcquireGLObjects
    /// \param num_objects  number of objects
    /// \param mem_objects mem object arrary
    /// \param num_events_in_wait_list num events in wait list
    /// \param event_wait_list event wait list array
    /// \param event Parameter for CLAPI_clEnqueueAcquireGLObjects
    /// \return ret code of real function
    cl_int Create(
        cl_command_queue  command_queue,
        cl_uint num_objects,
        const cl_mem* mem_objects,
        cl_uint num_events_in_wait_list,
        const cl_event* event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueAcquireGLObjects(const CLAPI_clEnqueueAcquireGLObjects& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueAcquireGLObjects& operator = (const CLAPI_clEnqueueAcquireGLObjects& obj);

private:
    cl_uint  m_num_objects; ///< parameter for clEnqueueNativeKernel
    cl_mem*  m_mem_objects; ///< parameter for clEnqueueNativeKernel
    cl_event m_event;       ///< parameter for clEnqueueNativeKernel
    cl_int   m_retVal;      ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueReleaseGLObjects
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueReleaseGLObjects : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueReleaseGLObjects() {}

    /// Destructor
    ~CLAPI_clEnqueueReleaseGLObjects()
    {
        if (m_mem_objects != NULL)
        {
            FreeArray(m_mem_objects);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_objects, m_num_objects) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueReleaseGLObjects
    /// \param command_queue Parameter for CLAPI_clEnqueueReleaseGLObjects
    /// \param num_objects  number of objects
    /// \param mem_objects mem object arrary
    /// \param num_events_in_wait_list num events in wait list
    /// \param event_wait_list event wait list array
    /// \param event Parameter for CLAPI_clEnqueueReleaseGLObjects
    /// \return ret code of real function
    cl_int Create(
        cl_command_queue  command_queue,
        cl_uint num_objects,
        const cl_mem* mem_objects,
        cl_uint num_events_in_wait_list,
        const cl_event* event_wait_list,
        cl_event*   event);


private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueReleaseGLObjects(const CLAPI_clEnqueueReleaseGLObjects& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueReleaseGLObjects& operator = (const CLAPI_clEnqueueReleaseGLObjects& obj);

private:
    cl_uint  m_num_objects; ///< parameter for clEnqueueNativeKernel
    cl_mem*  m_mem_objects; ///< parameter for clEnqueueNativeKernel
    cl_event m_event;       ///< parameter for clEnqueueNativeKernel
    cl_int   m_retVal;      ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueWaitForEvents
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueWaitForEvents : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clEnqueueWaitForEvents()
    {

    }

    /// Destructor
    ~CLAPI_clEnqueueWaitForEvents()
    {

    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_events << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_event_list);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueWaitForEvents
    /// \param ullStartTime API start time
    /// \param ullEndTime API end time
    /// \param command_queue Parameter for CLAPI_clEnqueueWaitForEvents
    /// \param num_events Parameter for CLAPI_clEnqueueWaitForEvents
    /// \param event_list Parameter for CLAPI_clEnqueueWaitForEvents
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_uint  num_events,
        const cl_event*   event_list,
        cl_int retVal);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueWaitForEvents(const CLAPI_clEnqueueWaitForEvents& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueWaitForEvents& operator = (const CLAPI_clEnqueueWaitForEvents& obj);

private:
    cl_command_queue      m_command_queue;   ///< parameter for clEnqueueWaitForEvents
    cl_uint               m_num_events;      ///< parameter for clEnqueueWaitForEvents
    const cl_event*       m_event_wait_list; ///< parameter for clEnqueueWaitForEvents
    std::vector<cl_event> m_event_list;      ///< parameter for clEnqueueWaitForEvents
    cl_int                m_retVal;          ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueBarrier
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueBarrier : public CLAPIBase
{
public:
    /// Constructor
    CLAPI_clEnqueueBarrier() {}

    /// Destructor
    ~CLAPI_clEnqueueBarrier() {}

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue);
        return ss.str();
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueBarrier
    /// \param ullStartTime start api call time stamp
    /// \param ullEndTime end api call time stamp
    /// \param command_queue Parameter for CLAPI_clEnqueueBarrier
    /// \param retVal return value
    void Create(
        ULONGLONG ullStartTime,
        ULONGLONG ullEndTime,
        cl_command_queue  command_queue,
        cl_int retVal);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueBarrier(const CLAPI_clEnqueueBarrier& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueBarrier& operator = (const CLAPI_clEnqueueBarrier& obj);

private:
    cl_command_queue  m_command_queue; ///< parameter for clEnqueueBarrier
    cl_int            m_retVal;        ///< return value
};

#ifdef _WIN32
//------------------------------------------------------------------------------------
/// clEnqueueAcquireD3D10ObjectsKHR
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueAcquireD3D10ObjectsKHR : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueAcquireD3D10ObjectsKHR() {}

    /// Destructor
    ~CLAPI_clEnqueueAcquireD3D10ObjectsKHR()
    {
        if (m_mem_objects != NULL)
        {
            FreeArray(m_mem_objects);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_objects, m_num_objects) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueAcquireD3D10ObjectsKHR
    /// \param command_queue Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    /// \param num_objects Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    /// \param mem_objects Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    /// \param event_wait_list Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    /// \param event Parameter for CLAPI_clEnqueueAcquireD3D10ObjectsKHR
    cl_int Create(
        cl_command_queue command_queue,
        cl_uint num_objects,
        const cl_mem* mem_objects,
        cl_uint num_events_in_wait_list,
        const cl_event* event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueAcquireD3D10ObjectsKHR(const CLAPI_clEnqueueAcquireD3D10ObjectsKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueAcquireD3D10ObjectsKHR& operator = (const CLAPI_clEnqueueAcquireD3D10ObjectsKHR& obj);

private:
    cl_uint  m_num_objects; ///< parameter for clEnqueueAcquireD3D10ObjectsKHR
    cl_mem*  m_mem_objects; ///< parameter for clEnqueueAcquireD3D10ObjectsKHR
    cl_event m_event;       ///< parameter for clEnqueueAcquireD3D10ObjectsKHR
    cl_int   m_retVal;      ///< return value
};

//------------------------------------------------------------------------------------
/// clEnqueueReleaseD3D10ObjectsKHR
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueReleaseD3D10ObjectsKHR : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueReleaseD3D10ObjectsKHR() {}

    /// Destructor
    ~CLAPI_clEnqueueReleaseD3D10ObjectsKHR()
    {
        if (m_mem_objects != NULL)
        {
            FreeArray(m_mem_objects);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        std::ostringstream ss;
        ss << CLStringUtils::GetErrorString(m_retVal);
        return ss.str();
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_objects, m_num_objects) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueReleaseD3D10ObjectsKHR
    /// \param command_queue Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    /// \param num_objects Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    /// \param mem_objects Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    /// \param num_objects Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    /// \param event_wait_list Parameter for CLAPI_clEnqueueReleaseD3D10ObjectsKHR
    cl_int Create(
        cl_command_queue command_queue,
        cl_uint num_objects,
        const cl_mem* mem_objects,
        cl_uint num_events_in_wait_list,
        const cl_event* event_wait_list,
        cl_event*   event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueReleaseD3D10ObjectsKHR(const CLAPI_clEnqueueReleaseD3D10ObjectsKHR& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueReleaseD3D10ObjectsKHR& operator = (const CLAPI_clEnqueueReleaseD3D10ObjectsKHR& obj);

private:
    cl_uint  m_num_objects; ///< parameter for clEnqueueReleaseD3D10ObjectsKHR
    cl_mem*  m_mem_objects; ///< parameter for clEnqueueReleaseD3D10ObjectsKHR
    cl_event m_event;       ///< parameter for clEnqueueReleaseD3D10ObjectsKHR
    cl_int   m_retVal;      ///< return value
};
#endif

//------------------------------------------------------------------------------------
/// clEnqueueFillBuffer
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueFillBuffer : public CLEnqueueData
{
public:
    /// Constructor
    CLAPI_clEnqueueFillBuffer() { }

    /// Destructor
    ~CLAPI_clEnqueueFillBuffer() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_buffer) << s_strParamSeparator
           << StringUtils::ToHexString(m_pattern) << s_strParamSeparator
           << m_pattern_size << s_strParamSeparator
           << m_offset << s_strParamSeparator
           << m_size << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_size;
    }

    /// Save the parameter values, return value and time stamps of CLAPI_clEnqueueFillBuffer
    /// \param command_queue Parameter for CLAPI_clEnqueueFillBuffer
    /// \param buffer Parameter for CLAPI_clEnqueueFillBuffer
    /// \param pattern Parameter for CLAPI_clEnqueueFillBuffer
    /// \param pattern_size Parameter for CLAPI_clEnqueueFillBuffer
    /// \param offset Parameter for CLAPI_clEnqueueFillBuffer
    /// \param size Parameter for CLAPI_clEnqueueFillBuffer
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueFillBuffer
    /// \param event_wait_list Parameter for CLAPI_clEnqueueFillBuffer
    /// \param event Parameter for CLAPI_clEnqueueFillBuffer
    cl_int Create(cl_command_queue command_queue,
                  cl_mem           buffer,
                  const void*      pattern,
                  size_t           pattern_size,
                  size_t           offset,
                  size_t           size,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueFillBuffer(const CLAPI_clEnqueueFillBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueFillBuffer& operator = (const CLAPI_clEnqueueFillBuffer& obj);

    cl_mem      m_buffer;       ///< parameter for clEnqueueFillBuffer
    const void* m_pattern;      ///< parameter for clEnqueueFillBuffer
    size_t      m_pattern_size; ///< parameter for clEnqueueFillBuffer
    size_t      m_offset;       ///< parameter for clEnqueueFillBuffer
    size_t      m_size;         ///< parameter for clEnqueueFillBuffer
    cl_event    m_event;        ///< parameter for clEnqueueFillBuffer
    cl_int      m_retVal;       ///< return value of clEnqueueFillBuffer
};

//------------------------------------------------------------------------------------
/// clEnqueueFillImage
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueFillImage : public CLEnqueueData
{
public:
    /// Constructor
    CLAPI_clEnqueueFillImage() { }

    /// Destructor
    ~CLAPI_clEnqueueFillImage() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_image) << s_strParamSeparator
           << StringUtils::ToHexString(m_fill_color) << s_strParamSeparator  //TODO: can this be decoded?
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_origin_null, m_origin), 3) << s_strParamSeparator
           << CLStringUtils::GetNDimString(REPLACEDNULLVAL(m_region_null, m_region), 3) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_region[0] * m_region[1] * m_region[2] * GetImageFormatSizeInByte(m_format);
    }

    /// Save the parameter values, return value and time stamps of clEnqueueFillImage
    /// \param command_queue Parameter for CLAPI_clEnqueueFillImage
    /// \param image Parameter for CLAPI_clEnqueueFillImage
    /// \param fill_color Parameter for CLAPI_clEnqueueFillImage
    /// \param origin Parameter for CLAPI_clEnqueueFillImage
    /// \param region Parameter for CLAPI_clEnqueueFillImage
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueFillImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueFillImage
    /// \param event Parameter for CLAPI_clEnqueueFillImage
    cl_int Create(cl_command_queue command_queue,
                  cl_mem           image,
                  const void*      fill_color,
                  const size_t     origin[3] ,
                  const size_t     region[3] ,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueFillImage(const CLAPI_clEnqueueFillImage& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueFillImage& operator = (const CLAPI_clEnqueueFillImage& obj);

    cl_mem          m_image;       ///< parameter for clEnqueueFillImage
    const void*     m_fill_color;  ///< parameter for clEnqueueFillImage
    size_t          m_origin[3];   ///< parameter for clEnqueueFillImage
    size_t          m_region[3];   ///< parameter for clEnqueueFillImage
    cl_event        m_event;       ///< parameter for clEnqueueFillImage
    cl_int          m_retVal;      ///< return value of clEnqueueFillImage
    cl_image_format m_format;      ///< image format
    bool            m_origin_null; ///< flag indicating that m_origin was NULL
    bool            m_region_null; ///< flag indicating that m_region was NULL


};

//------------------------------------------------------------------------------------
/// clEnqueueMigrateMemObjects
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMigrateMemObjects : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueMigrateMemObjects() { }

    /// Destructor
    ~CLAPI_clEnqueueMigrateMemObjects()
    {
        if (m_mem_objects != NULL)
        {
            FreeArray(m_mem_objects);
        }
    }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_mem_objects << s_strParamSeparator
           << CLStringUtils::GetHandlesString(m_mem_objects, m_num_mem_objects) << s_strParamSeparator
           << CLStringUtils::GetMemMigrationFlagsString(m_flags) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values, return value and time stamps of clEnqueueMigrateMemObjects
    /// \param command_queue Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param num_mem_objects Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param mem_objects Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param flags Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param event_wait_list Parameter for CLAPI_clEnqueueMigrateMemObjects
    /// \param event Parameter for CLAPI_clEnqueueMigrateMemObjects
    cl_int Create(cl_command_queue       command_queue,
                  cl_uint                num_mem_objects,
                  const cl_mem*          mem_objects,
                  cl_mem_migration_flags flags,
                  cl_uint                num_events_in_wait_list,
                  const cl_event*        event_wait_list,
                  cl_event*              event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueMigrateMemObjects(const CLAPI_clEnqueueMigrateMemObjects& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueMigrateMemObjects& operator = (const CLAPI_clEnqueueMigrateMemObjects& obj);

    cl_uint                m_num_mem_objects; ///< parameter for clEnqueueMigrateMemObjects
    cl_mem*                m_mem_objects;     ///< parameter for clEnqueueMigrateMemObjects
    cl_mem_migration_flags m_flags;           ///< parameter for clEnqueueMigrateMemObjects
    cl_event               m_event;           ///< parameter for clEnqueueMigrateMemObjects
    cl_int                 m_retVal;          ///< return value of clEnqueueMigrateMemObjects
};

//------------------------------------------------------------------------------------
/// clEnqueueMarkerWithWaitList
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueMarkerWithWaitList : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueMarkerWithWaitList() { }

    /// Destructor
    ~CLAPI_clEnqueueMarkerWithWaitList() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }


    /// Save the parameter values, return value and time stamps of clEnqueueMarkerWithWaitList
    /// \param command_queue Parameter for CLAPI_clEnqueueMarkerWithWaitList
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueFillImage
    /// \param event_wait_list Parameter for CLAPI_clEnqueueFillImage
    /// \param event Parameter for CLAPI_clEnqueueFillImage
    cl_int Create(cl_command_queue command_queue,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueMarkerWithWaitList(const CLAPI_clEnqueueMarkerWithWaitList& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueMarkerWithWaitList& operator = (const CLAPI_clEnqueueMarkerWithWaitList& obj);

    cl_event         m_event;  ///< parameter for clEnqueueMarkerWithWaitList
    cl_int           m_retVal; ///< return value of clEnqueueMarkerWithWaitList
};

//------------------------------------------------------------------------------------
/// clEnqueueBarierWithWaitList
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueBarrierWithWaitList : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueBarrierWithWaitList() { }

    /// Destructor
    ~CLAPI_clEnqueueBarrierWithWaitList() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueBarrierWithWaitList
    /// \param command_queue Parameter for CLAPI_clEnqueueBarrierWithWaitList
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueBarrierWithWaitList
    /// \param event_wait_list Parameter for CLAPI_clEnqueueBarrierWithWaitList
    /// \param event Parameter for CLAPI_clEnqueueBarrierWithWaitList
    cl_int Create(cl_command_queue command_queue,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueBarrierWithWaitList(const CLAPI_clEnqueueBarrierWithWaitList& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueBarrierWithWaitList& operator = (const CLAPI_clEnqueueBarrierWithWaitList& obj);

    cl_event         m_event;  ///< parameter for clEnqueueBarrierWithWaitList
    cl_int           m_retVal; ///< return value of clEnqueueBarrierWithWaitList
};

//------------------------------------------------------------------------------------
/// clEnqueueSVMFree
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueSVMFree : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueSVMFree() { }

    /// Destructor
    ~CLAPI_clEnqueueSVMFree() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << m_num_svm_pointers << s_strParamSeparator
           << CLStringUtils::GetPointerListString(m_svm_pointers, m_pointer_list) << s_strParamSeparator
           << StringUtils::ToHexString(m_pfn_free_func) << s_strParamSeparator
           << StringUtils::ToHexString(m_user_data) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueSVMFree
    /// \param command_queue Parameter for CLAPI_clEnqueueSVMFree
    /// \param num_svm_pointers Parameter for CLAPI_clEnqueueSVMFree
    /// \param svm_pointers Parameter for CLAPI_clEnqueueSVMFree
    /// \param pfn_free_func Parameter for CLAPI_clEnqueueSVMFree
    /// \param user_data Parameter for CLAPI_clEnqueueSVMFree
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueSVMFree
    /// \param event_wait_list Parameter for CLAPI_clEnqueueSVMFree
    /// \param event Parameter for CLAPI_clEnqueueSVMFree
    /// \param isExtension flag indicating whether the application called the extension API (clEnqueueSVMFreeAMD) or the real API (clEnqueueSVMFree)
    cl_int Create(cl_command_queue command_queue,
                  cl_uint          num_svm_pointers,
                  void*            svm_pointers[],
                  void (CL_CALLBACK* pfn_free_func)(cl_command_queue, cl_uint, void* [], void*),
                  void*            user_data,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event,
                  bool             isExtension = false);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueSVMFree(const CLAPI_clEnqueueSVMFree& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueSVMFree& operator = (const CLAPI_clEnqueueSVMFree& obj);

    cl_uint            m_num_svm_pointers; ///< parameter for clEnqueueSVMFree
    void**             m_svm_pointers;     ///< parameter for clEnqueueSVMFree
    std::vector<void*> m_pointer_list;     ///< parameter for clEnqueueSVMFree
    void (CL_CALLBACK* m_pfn_free_func)(cl_command_queue, cl_uint, void* [], void*); ///< parameter for clEnqueueSVMFree
    void*              m_user_data;        ///< parameter for clEnqueueSVMFree
    cl_event           m_event;            ///< parameter for clEnqueueSVMFree
    cl_int             m_retVal;           ///< return value of clEnqueueSVMFree
};

//------------------------------------------------------------------------------------
/// clEnqueueSVMMemcpy
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueSVMMemcpy : public CLEnqueueData
{
public:
    /// Constructor
    CLAPI_clEnqueueSVMMemcpy() { }

    /// Destructor
    ~CLAPI_clEnqueueSVMMemcpy() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_copy) << s_strParamSeparator
           << StringUtils::ToHexString(m_dst_ptr) << s_strParamSeparator
           << StringUtils::ToHexString(m_src_ptr) << s_strParamSeparator
           << m_size << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueSVMMemcpy
    /// \param command_queue Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param blocking_copy Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param dst_ptr Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param src_ptr Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param size Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param event_wait_list Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param event Parameter for CLAPI_clEnqueueSVMMemcpy
    /// \param isExtension flag indicating whether the application called the extension API (clEnqueueSVMMemcpyAMD) or the real API (clEnqueueSVMFree)
    cl_int Create(cl_command_queue command_queue,
                  cl_bool          blocking_copy,
                  void*            dst_ptr,
                  const void*      src_ptr,
                  size_t           size,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event,
                  bool             isExtension = false);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_size;
    }


private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueSVMMemcpy(const CLAPI_clEnqueueSVMMemcpy& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueSVMMemcpy& operator = (const CLAPI_clEnqueueSVMMemcpy& obj);

    cl_bool     m_blocking_copy; ///< parameter for clEnqueueSVMMemcpy
    void*       m_dst_ptr;       ///< parameter for clEnqueueSVMMemcpy
    const void* m_src_ptr;       ///< parameter for clEnqueueSVMMemcpy
    size_t      m_size;          ///< parameter for clEnqueueSVMMemcpy
    cl_event    m_event;         ///< parameter for clEnqueueSVMMemcpy
    cl_int      m_retVal;        ///< return value of clEnqueueSVMMemcpy
};

//------------------------------------------------------------------------------------
/// clEnqueueSVMMemFill
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueSVMMemFill : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueSVMMemFill() { }

    /// Destructor
    ~CLAPI_clEnqueueSVMMemFill() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_svm_ptr) << s_strParamSeparator
           << StringUtils::ToHexString(m_pattern) << s_strParamSeparator
           << m_pattern_size << s_strParamSeparator
           << m_size << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueSVMMemFill
    /// \param command_queue Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param svm_ptr Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param pattern Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param pattern_size Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param size Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param event_wait_list Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param event Parameter for CLAPI_clEnqueueSVMMemFill
    /// \param isExtension flag indicating whether the application called the extension API (clEnqueueSVMMemFillAMD) or the real API (clEnqueueSVMMemFill)
    cl_int Create(cl_command_queue command_queue,
                  void*            svm_ptr,
                  const void*      pattern,
                  size_t           pattern_size,
                  size_t           size,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event,
                  bool             isExtension = false);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_size;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueSVMMemFill(const CLAPI_clEnqueueSVMMemFill& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueSVMMemFill& operator = (const CLAPI_clEnqueueSVMMemFill& obj);

    void*       m_svm_ptr;       ///< parameter for clEnqueueSVMMemcpy
    const void* m_pattern;       ///< parameter for clEnqueueSVMMemcpy
    size_t      m_pattern_size;  ///< parameter for clEnqueueSVMMemcpy
    size_t      m_size;          ///< parameter for clEnqueueSVMMemcpy
    cl_event    m_event;         ///< parameter for clEnqueueSVMMemcpy
    cl_int      m_retVal;        ///< return value of clEnqueueSVMMemcpy
};

//------------------------------------------------------------------------------------
/// clEnqueueSVMMap
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueSVMMap : public CLEnqueueData
{
public:
    /// Constructor
    CLAPI_clEnqueueSVMMap() { }

    /// Destructor
    ~CLAPI_clEnqueueSVMMap() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << CLStringUtils::GetBoolString(m_blocking_map) << s_strParamSeparator
           << CLStringUtils::GetMapFlagsString(m_flags) << s_strParamSeparator
           << StringUtils::ToHexString(m_svm_ptr) << s_strParamSeparator
           << m_size << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueSVMMap
    /// \param command_queue Parameter for CLAPI_clEnqueueSVMMap
    /// \param blocking_map Parameter for CLAPI_clEnqueueSVMMap
    /// \param flags Parameter for CLAPI_clEnqueueSVMMap
    /// \param svm_ptr Parameter for CLAPI_clEnqueueSVMMap
    /// \param size Parameter for CLAPI_clEnqueueSVMMap
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueSVMMap
    /// \param event_wait_list Parameter for CLAPI_clEnqueueSVMMap
    /// \param event Parameter for CLAPI_clEnqueueSVMMap
    /// \param isExtension flag indicating whether the application called the extension API (clEnqueueSVMMapAMD) or the real API (clEnqueueSVMMap)
    cl_int Create(cl_command_queue command_queue,
                  cl_bool          blocking_map,
                  cl_map_flags     flags,
                  void*            svm_ptr,
                  size_t           size,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event,
                  bool             isExtension = false);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return m_size;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueSVMMap(const CLAPI_clEnqueueSVMMap& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueSVMMap& operator = (const CLAPI_clEnqueueSVMMap& obj);

    cl_bool      m_blocking_map;  ///< parameter for clEnqueueSVMMap
    cl_map_flags m_flags;         ///< parameter for clEnqueueSVMMap
    void*        m_svm_ptr;       ///< parameter for clEnqueueSVMMap
    size_t       m_size;          ///< parameter for clEnqueueSVMMap
    cl_event     m_event;         ///< parameter for clEnqueueSVMMap
    cl_int       m_retVal;        ///< return value of clEnqueueSVMMap
};

//------------------------------------------------------------------------------------
/// clEnqueueSVMUnmap
//------------------------------------------------------------------------------------
class CLAPI_clEnqueueSVMUnmap : public CLEnqueueOther
{
public:
    /// Constructor
    CLAPI_clEnqueueSVMUnmap() { }

    /// Destructor
    ~CLAPI_clEnqueueSVMUnmap() { }

    /// Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    bool GetAPISucceeded() const
    {
        return m_retVal == CL_SUCCESS;
    }

    /// get return value string
    /// \return string representation of the return value;
    std::string GetRetString()
    {
        return CLStringUtils::GetErrorString(m_retVal);
    }

    /// To String
    /// \return string representation of the API
    std::string ToString()
    {
        std::ostringstream ss;
        ss << StringUtils::ToHexString(m_command_queue) << s_strParamSeparator
           << StringUtils::ToHexString(m_svm_ptr) << s_strParamSeparator
           << m_num_events_in_wait_list << s_strParamSeparator
           << CLStringUtils::GetEventListString(m_event_wait_list, m_vecEvent_wait_list) << s_strParamSeparator
           << CLStringUtils::GetEventString(m_event);
        return ss.str();
    }

    /// Save the parameter values and return value of clEnqueueSVMUnmap
    /// \param command_queue Parameter for CLAPI_clEnqueueSVMUnmap
    /// \param svm_ptr Parameter for CLAPI_clEnqueueSVMUnmap
    /// \param num_events_in_wait_list Parameter for CLAPI_clEnqueueSVMUnmap
    /// \param event_wait_list Parameter for CLAPI_clEnqueueSVMUnmap
    /// \param event Parameter for CLAPI_clEnqueueSVMUnmap
    /// \param isExtension flag indicating whether the application called the extension API (clEnqueueSVMUnmapAMD) or the real API (clEnqueueSVMUnmap)
    cl_int Create(cl_command_queue command_queue,
                  void*            svm_ptr,
                  cl_uint          num_events_in_wait_list,
                  const cl_event*  event_wait_list,
                  cl_event*        event,
                  bool             isExtension = false);

    /// Get data transfer size in byte
    /// \return data transfer size in byte
    size_t GetDataSize() const
    {
        return 0;
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPI_clEnqueueSVMUnmap(const CLAPI_clEnqueueSVMUnmap& obj);

    /// Disable assignment operator
    /// \param[in] obj the input object
    /// \return a reference of the object
    CLAPI_clEnqueueSVMUnmap& operator = (const CLAPI_clEnqueueSVMUnmap& obj);

    void*        m_svm_ptr;       ///< parameter for clEnqueueSVMUnmap
    cl_event     m_event;         ///< parameter for clEnqueueSVMUnmap
    cl_int       m_retVal;        ///< return value of clEnqueueSVMUnmap
};

// @}

#endif //_CL_ENQUEUE_API_DEFS_H_
