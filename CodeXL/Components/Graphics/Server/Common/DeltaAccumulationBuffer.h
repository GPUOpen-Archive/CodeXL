//==============================================================================
// Copyright (c) 2010-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A class that stores an original buffer and a secondary copy
///         to which BufferDeltas can be applied. This helps us store data
///         across multiple map/unmap calls.
//==============================================================================

#ifndef GPS_DELTA_ACCUMULATION_BUFFER_H_
#define GPS_DELTA_ACCUMULATION_BUFFER_H_

#include "BufferDelta.h"

/// A class that stores an original buffer and a secondary copy
/// to which BufferDeltas can be applied. This helps us store data
/// across multiple map/unmap calls.
class DeltaAccumulationBuffer
{
public:

    /// Default constructor
    DeltaAccumulationBuffer();

    /// destructor
    ~DeltaAccumulationBuffer();

    /// Copy constructor
    /// \param srcBuffer The delta accumulation buffer to copy.
    DeltaAccumulationBuffer(const DeltaAccumulationBuffer& srcBuffer);

    /// Sets the baseline buffer and size.
    /// \param pBuffer The original contents of the buffer
    /// \param bufferSize The number of bytes in the buffer
    void SetBuffer(const char* pBuffer, size_t bufferSize);

    /// Restores the accumulated delta buffer back to the original state.
    void Restore();

    /// Applys a delta to the delta accumulation buffer.
    /// \param delta The delta to apply.
    void ApplyDelta(BufferDelta& delta);

    /// Calculate the delta between us and incoming buffer
    /// \param pBuffer Input buffer
    /// \param delta Output delta
    void CalculateDelta(const char* pBuffer, BufferDelta& delta);

    /// Stores the size of the original buffer, which is also the size of the accumulated delta buffer.
    size_t m_bufferSize;

    /// The original buffer that was recorded before any deltas were applied.
    char* m_pOriginalBuffer;

    /// The buffer that has deltas applied to and from which deltas are calculated.
    char* m_pAccumulatedDeltaBuffer;

private:

    /// Deletes the original and delta accumulation buffers to prevent
    /// memory leaks.
    void DeleteBuffers();
};
#endif //GPS_DELTA_ACCUMULATION_BUFFER_H_
