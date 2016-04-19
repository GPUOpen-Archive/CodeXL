//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a buffer call.
//==============================================================================

#include <CL/opencl.h>
#include <stdlib.h>

#include "CLBuffer.h"

bool CLBuffer::IsEqual(const cl_mem& buffer)
{
    return (buffer == m_buffer);
}

cl_mem CLBuffer::GetBuffer() const
{
    return m_buffer;
}

size_t CLBuffer::GetBufferSize() const
{
    return m_bufferSize;
}
