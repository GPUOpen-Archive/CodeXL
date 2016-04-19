//==============================================================================
// Copyright (c) 2010-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A class that can calculate, store, and reapply a delta between
///         two buffers of equal size.
//==============================================================================

#ifndef GPS_BUFFER_DELTA_H_
#define GPS_BUFFER_DELTA_H_

#include <vector>

#define APPLY_DELTA
//#define VERIFY_DELTA_RESULTS

//#define USE_VECTOR_FOR_DATA

/// A class that can calculate, store, and reapply a delta between
/// two buffers of equal size.
class BufferDelta
{
public:
    //===============================================================================================
    /// Given two buffers, extracts the differences.
    /// Buffers must be the same size.
    /// \param[in] baseBuffer Original buffer data
    /// \param[in] diffBuffer Modified buffer
    /// \param[in] numBytes size of the buffer in bytes
    //===============================================================================================
    BufferDelta(char* baseBuffer, char* diffBuffer, size_t numBytes);

    //===============================================================================================
    /// Default constructor
    //===============================================================================================
    BufferDelta(void);

    //===============================================================================================
    /// Destructor
    //===============================================================================================
    ~BufferDelta(void);

    //===============================================================================================
    /// Given two buffers, extracts the differences.
    /// Buffers must be the same size.
    /// \param[in] baseBuffer Original buffer data
    /// \param[in] diffBuffer Modified buffer
    /// \param[in] numBytes size of the buffer in bytes
    //===============================================================================================
    void CalculateDelta(const char* baseBuffer, const char* diffBuffer, const size_t numBytes);

    //===============================================================================================
    /// Apply deltas to a buffer.
    /// Buffers must be the same size.
    /// \param[in] pBuffer buffer where to apply the deltas
    //===============================================================================================
    void ApplyDelta(char* pBuffer);

private:

    //===============================================================================================
    /// Clears the data inside this delta
    //===============================================================================================
    void Clear();

#if defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)
    /// A copy of the buffer after the app has made changes to it.
    char* m_copiedDiffBuffer;

    /// The size of the copied buffer
    size_t m_copiedDiffBufferSize;
#endif // defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)

#ifdef USE_VECTOR_FOR_DATA
    /// list of chunks (back to back)
    std::vector<char> m_data;
#else
    /// manually managed array of diffs
    char* m_pDiffs;
#endif // USE_VECTOR_FOR_DATA

    /// list of offsets to chucks
    std::vector<size_t> m_offset;

    /// list of chuck sizes
    std::vector<size_t> m_size;
};

#endif // GPS_BUFFER_DELTA_H_