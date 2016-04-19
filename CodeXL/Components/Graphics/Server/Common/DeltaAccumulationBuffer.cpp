//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A class that stores an original buffer and a secondary copy
///         to which BufferDeltas can be applied. This helps us store data
///         across multiple map/unmap calls.
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#elif defined (_LINUX)
    #include "WinDefs.h"
#endif
#include "DeltaAccumulationBuffer.h"
#include "Logger.h"

#ifdef USE_MAP_STREAMLOG
    long g_totalResourceBufferMemory = 0 ;
#endif

//===================================================================
DeltaAccumulationBuffer::DeltaAccumulationBuffer()
    : m_bufferSize(0),
      m_pOriginalBuffer(NULL),
      m_pAccumulatedDeltaBuffer(NULL)
{
}

//===================================================================
DeltaAccumulationBuffer::~DeltaAccumulationBuffer()
{
    DeleteBuffers();
}

//===================================================================
DeltaAccumulationBuffer::DeltaAccumulationBuffer(const DeltaAccumulationBuffer& srcBuffer)
{
    m_bufferSize = srcBuffer.m_bufferSize;

    if (m_bufferSize > 0)
    {
        m_pAccumulatedDeltaBuffer = new char[m_bufferSize];
        m_pOriginalBuffer = new char[m_bufferSize];

#ifdef USE_MAP_STREAMLOG
        g_totalResourceBufferMemory += (long)m_bufferSize * 2;
#endif

        memcpy(m_pOriginalBuffer, srcBuffer.m_pOriginalBuffer, m_bufferSize);
        memcpy(m_pAccumulatedDeltaBuffer, srcBuffer.m_pAccumulatedDeltaBuffer, m_bufferSize);
    }
    else
    {
        m_pOriginalBuffer = NULL;
        m_pAccumulatedDeltaBuffer = NULL;
    }
}

//===================================================================
void DeltaAccumulationBuffer::SetBuffer(const char* pBuffer, size_t bufferSize)
{
    DeleteBuffers();

    m_bufferSize = bufferSize;

    {
        try
        {
            m_pOriginalBuffer = new char[m_bufferSize];

#ifdef USE_MAP_STREAMLOG
            g_totalResourceBufferMemory += (long)m_bufferSize;
#endif
        }
        catch (std::bad_alloc)
        {
            Log(logASSERT, "Ran out of memory; could not allocate base delta buffer.\n");
            m_pOriginalBuffer = NULL;
            m_bufferSize = 0;
            return;
        }

        try
        {
            m_pAccumulatedDeltaBuffer = new char[m_bufferSize];

#ifdef USE_MAP_STREAMLOG
            g_totalResourceBufferMemory += (long)m_bufferSize;
#endif
        }
        catch (std::bad_alloc)
        {
            Log(logASSERT, "Ran out of memory; could not allocate delta accumulation buffer.\n");
            m_pAccumulatedDeltaBuffer = NULL;
            delete [] m_pOriginalBuffer;
            m_pOriginalBuffer = NULL;
            m_bufferSize = 0;
            return;
        }

        memcpy(m_pOriginalBuffer, pBuffer, m_bufferSize);
        memcpy(m_pAccumulatedDeltaBuffer, pBuffer, m_bufferSize);
    }

#ifdef USE_MAP_STREAMLOG
    StreamLog::Get() << "DeltaAccumulationBuffer: g_totalResourceBufferMemory: " << g_totalResourceBufferMemory << " bytes" << "\n";
#endif
}

//===================================================================
void DeltaAccumulationBuffer::Restore()
{
    {
        memcpy(m_pAccumulatedDeltaBuffer, m_pOriginalBuffer, m_bufferSize);
    }
}

//===================================================================
void DeltaAccumulationBuffer::ApplyDelta(BufferDelta& delta)
{
    delta.ApplyDelta(m_pAccumulatedDeltaBuffer);
}

//===================================================================
void DeltaAccumulationBuffer::CalculateDelta(const char* pBuffer, BufferDelta& delta)
{
    {
        delta.CalculateDelta(m_pAccumulatedDeltaBuffer, pBuffer, this->m_bufferSize);
    }
}

//===================================================================
void DeltaAccumulationBuffer::DeleteBuffers()
{
    m_bufferSize = 0;

    if (m_pOriginalBuffer != NULL)
    {
        delete [] m_pOriginalBuffer;
        m_pOriginalBuffer = NULL;
    }

    if (m_pAccumulatedDeltaBuffer != NULL)
    {
        delete [] m_pAccumulatedDeltaBuffer;
        m_pAccumulatedDeltaBuffer = NULL;
    }
}
