//==============================================================================
// Copyright (c) 2009-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
//==============================================================================

#pragma once

#ifndef GDT_MEMORY_H
#define GDT_MEMORY_H

#include "GDT_MemUtils.h"

/// Memory utility classes.
namespace GDT_Memory
{
/// A memory buffer class that automatically resizes for streaming writes.
class GDT_MemoryBuffer
{
public:
    /// Default constructor.
    GDT_MemoryBuffer();

    /// Destructor.
    virtual ~GDT_MemoryBuffer();

    /// Re-allocate the memory buffer to the specified size. The contents of the buffer will be retained upto the new size.
    /// \param[in]    nSize    The new buffer size.
    /// \return                True if successful, otherwise false.
    bool ReAlloc(size_t nSize = 0);

    /// Get the size of buffer in use.
    /// \return                The size of buffer in use.
    size_t GetSize(void) const { return m_nSize; }

    /// Get a pointer to the buffer data.
    /// \return                A pointer to the buffer data.
    char* GetBuffer(void) const { return m_pBuffer; }

    /// Get the current offset within the buffer for data writing.
    /// \return                The buffer size.
    size_t GetOffset(void) const { return m_nOffset; }

    /// Write data to the buffer.
    /// \param[in]    pData    The data to write to the buffer.
    /// \param[in]    nSize    The buffer size.
    /// \return                The number of bytes written.
    size_t Write(const void* pData, size_t nSize);

    /// Write data to the buffer.
    /// \param[in]    t        The data to write to the buffer.
    /// \return                The number of bytes written.
    template <class T> size_t Write(const T& t) { return Write(&t, sizeof(t)); };

    /// Write data to the buffer.
    /// \param[in]    t        The data to write to the buffer.
    /// \return                The number of bytes written.
    template <class T> size_t Write(const T* t) { return Write(t, sizeof(*t)); };

    /// Write data to the buffer at the specified offset.
    /// \param[in]    nOffset  The offset within the buffer at which to write.
    /// \param[in]    pData    The data to write to the buffer.
    /// \param[in]    nSize    The buffer size.
    /// \return                The number of bytes written.
    size_t WriteAt(size_t nOffset, const void* pData, size_t nSize);

    /// Write data to the buffer at the specified offset.
    /// \param[in]    nOffset  The offset within the buffer at which to write.
    /// \param[in]    t        The data to write to the buffer.
    /// \return                The number of bytes written.
    template <class T> size_t WriteAt(size_t nOffset, const T& t) { return WriteAt(nOffset, &t, sizeof(t)); };

    /// Write data to the buffer at the specified offset.
    /// \param[in]    nOffset  The offset within the buffer at which to write.
    /// \param[in]    t        The data to write to the buffer.
    /// \return                The number of bytes written.
    template <class T> size_t WriteAt(size_t nOffset, const T* t) { return WriteAt(nOffset, t, sizeof(*t)); };

    /// Pads buffer to word boundary padding with supplied byte.
    /// \param[in]    padByte  The byte to pad buffer with.
    /// \return                The number of bytes written.
    size_t PadToWord(const char padByte);

    /// Pads buffer to double word boundary padding with supplied byte.
    /// \param[in]    padByte  The byte to pad buffer with.
    /// \return                The number of bytes written.
    size_t PadToDoubleWord(const char padByte);

private:
    /// Get the size of buffer allocated.
    /// \return                The size of buffer allocated.
    size_t GetAllocSize(void) const { return m_nMemorySize; }

    /// Copy constructor. Declared private & not-implemented to prevent use.
    /// \param[in]    buffer   The buffer to copy.
    GDT_MemoryBuffer(const GDT_MemoryBuffer& buffer);

    /// Assignment operator. Declared private & not-implemented to prevent use.
    /// \param[in]    rhs      The buffer to duplicate.
    GDT_MemoryBuffer& operator = (const GDT_MemoryBuffer& rhs);

    char* m_pBuffer;                                      ///< Pointer to the buffer.
    size_t m_nSize;                                       ///< The size of buffer in use.
    size_t m_nOffset;                                     ///< The current offset within the buffer.
    size_t m_nMemorySize;                                 ///< The size of buffer allocated.

    size_t m_nMemoryAllocIncrements;                      ///< The size of buffer allocation increments.

    static const size_t m_nDefaultMemoryAllocIncrements;  ///< The default size of buffer allocation increments.
};
} // namespace GDT_Memory

#endif // GDT_MEMORY_H
