//==============================================================================
// Copyright (c) 2009-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
//==============================================================================

#ifdef _WIN32
    #ifndef NOMINMAX
        // This prevents window.h from inserting non-standard min & max macros.
        // Those macros blow up std::min & std::max template functions (which we want).
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif
#include <sys/types.h>
#include "GDT_Memory.h"
#include <cstring>
#include <malloc.h>
#include <algorithm>

using namespace GDT_Memory;

const size_t GDT_Memory::GDT_MemoryBuffer::m_nDefaultMemoryAllocIncrements = 4096;

GDT_Memory::GDT_MemoryBuffer::GDT_MemoryBuffer() :
    m_pBuffer(NULL), m_nSize(0), m_nOffset(0), m_nMemorySize(0), m_nMemoryAllocIncrements(m_nDefaultMemoryAllocIncrements)
{
}

// GDT_Memory::GDT_MemoryBuffer::GDT_MemoryBuffer( const GDT_MemoryBuffer& buffer ) :
//    m_pBuffer(NULL), m_nSize(0), m_nOffset(0), m_nMemorySize(0), m_nMemoryAllocIncrements(m_nDefaultMemoryAllocIncrements)
// {
//    if ( ReAlloc( buffer.GetSize() ) )
//    {
//      copy( buffer.GetBuffer(), buffer.GetSize() );
//    }
// }

GDT_Memory::GDT_MemoryBuffer::~GDT_MemoryBuffer()
{
}


bool GDT_Memory::GDT_MemoryBuffer::ReAlloc(size_t nSize)
{
    size_t nAllocSize = GetAllocSize() + nSize;
    size_t nRoundedAllocSize = (nAllocSize + (m_nMemoryAllocIncrements - 1)) & ~(m_nMemoryAllocIncrements - 1);

    char* pBuffer = (char*) realloc(GetBuffer(), nRoundedAllocSize);

    if (pBuffer != NULL)
    {
        m_pBuffer = pBuffer;
        m_nMemorySize = nRoundedAllocSize;

        return true;
    }

    return false;
}


size_t GDT_Memory::GDT_MemoryBuffer::Write(const void* pData, size_t nSize)
{
    if (pData != NULL && nSize > 0)
    {
        size_t nRemainder = m_nMemorySize - m_nOffset;

        if (nRemainder < nSize)
        {
            ReAlloc(nSize);
            nRemainder = m_nMemorySize - m_nOffset;
        }

        size_t nWriteLength = (nRemainder < nSize) ? nRemainder : nSize;

        memcpy(GetBuffer() + GetOffset(), pData, nWriteLength);

        m_nOffset += nWriteLength;

        return nWriteLength;
    }

    return 0;
}


size_t GDT_Memory::GDT_MemoryBuffer::WriteAt(size_t nOffset, const void* pData, size_t nSize)
{
    if (pData != NULL && nSize > 0)
    {
        size_t nTotalDataSize = (nSize + nOffset);

        if (nTotalDataSize > GetAllocSize())
        {
            ReAlloc(nTotalDataSize);
        }

        size_t nWriteLength = (nTotalDataSize > GetAllocSize()) ? (nTotalDataSize - GetAllocSize()) : nSize;

        memcpy(GetBuffer() + nOffset, pData, nWriteLength);

        m_nOffset = std::max(m_nOffset, nOffset + nWriteLength);

        return nWriteLength;
    }

    return 0;
}


// TODO: convert to template function to pad to sizeof(T)
size_t GDT_Memory::GDT_MemoryBuffer::PadToWord(const char padByte)
{
    if (GetOffset() & 0x01)
    {
        return Write(&padByte, 1);
    }

    return 0;
}


size_t GDT_Memory::GDT_MemoryBuffer::PadToDoubleWord(const char padByte)
{
    size_t i = 0;

    while (GetOffset() & 0x03)
    {
        i += Write(&padByte, 1);
    }

    return i;
}
