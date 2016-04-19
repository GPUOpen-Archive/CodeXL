//==============================================================================
// Copyright (c) 2014-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A simple memory buffer class which has functionality to detect buffer overruns
//==============================================================================

#if defined _WIN32
    #include "windows.h"
#else
    #include "SafeCRT.h"
#endif // _WIN32

#include <stdlib.h>
#include "MemoryBuffer.h"
#include "Logger.h"

/// The guard at the end of the buffer
static const UINT32 SENTINAL_VALUE = 0xdeadbeef;
/// Length of the guard at the end of the buffer
static const UINT32 SENTINAL_LENGTH = 4;

MemoryBuffer::MemoryBuffer()
    : m_pBuffer(NULL)
    , m_bufferSize(0)
{
}

MemoryBuffer::~MemoryBuffer()
{
    Free();
}

unsigned char* MemoryBuffer::Alloc(unsigned int bufferSize)
{
    if (m_pBuffer)
    {
        Free();
    }

    m_bufferSize = bufferSize;
    m_pBuffer = (unsigned char*)malloc(m_bufferSize + SENTINAL_LENGTH);

    if (m_pBuffer)
    {
        // write the sentinal value
        unsigned char* pBufferEnd = (unsigned char*)m_pBuffer + m_bufferSize;
        memcpy_s(pBufferEnd, 4, &SENTINAL_VALUE, 4);
    }

    return m_pBuffer;
}

void MemoryBuffer::Free(void)
{
    if (m_pBuffer)
    {
        free(m_pBuffer);
        m_bufferSize = 0;
    }
}

bool MemoryBuffer::BufferOverrun()
{
    unsigned char* pBufferEnd = (unsigned char*)m_pBuffer + m_bufferSize;
    UINT32 val;
    memcpy_s(&val, 4, pBufferEnd, 4);

    if (val != SENTINAL_VALUE)
    {
        return true;
    }

    return false;
}
