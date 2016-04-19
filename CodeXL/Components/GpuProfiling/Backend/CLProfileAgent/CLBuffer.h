//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a buffer call.
//==============================================================================

#ifndef _CL_BUFFER_H_
#define _CL_BUFFER_H_

/// \addtogroup CLContextManager
// @{

/// This class manages all the information in a buffer class
class CLBuffer
{
public:
    /// default constructor.
    CLBuffer(): m_buffer(NULL), m_bufferSize(0), m_flags(0), m_pHost(NULL) { }

    /// constructor.
    /// \param buffer     the CL buffer
    /// \param bufferSize the CL buffer size
    /// \param flags      flags used in clCreateBuffer
    /// \param pHost      the host pointer used in clCreateBuffer
    CLBuffer(const cl_mem buffer, size_t bufferSize, cl_mem_flags flags, void* pHost):
        m_buffer(buffer), m_bufferSize(bufferSize), m_flags(flags), m_pHost(pHost) { }

    /// destructor.
    ~CLBuffer() { m_buffer = NULL; m_pHost = NULL; }

    /// Check the stored buffer.
    /// \param buffer the CL buffer
    /// \return true if the stored buffer is equal to the param, false otherwise
    bool IsEqual(const cl_mem& buffer);

    /// get the stored buffer.
    /// \return the stored buffer
    cl_mem GetBuffer() const;

    /// get the buffer size of the buffer.
    /// \return the buffer size of the buffer
    size_t GetBufferSize() const;

private:
    // copy constructor.
    CLBuffer(const CLBuffer& CLBuffer);

    /// assignment operator.
    /// \param CLBuffer the rhs object
    /// \return a reference to the object
    CLBuffer& operator=(const CLBuffer& CLBuffer);

    cl_mem       m_buffer;      ///< the buffer pointer argument from the API call
    size_t       m_bufferSize;  ///< the buffer size argument from the API call
    cl_mem_flags m_flags;       ///< the flags argument from the API call
    void*        m_pHost;       ///< the host pointer argument from the API call

};

// @}

#endif // _CL_BUFFER_H_
