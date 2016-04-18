//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A class that can calculate, store, and reapply a delta between
///         two buffers of equal size.
//==============================================================================

#if defined (_LINUX)
    #include "WinDefs.h"
#endif
#include "BufferDelta.h"
#include <assert.h>
#include "Logger.h"

//===============================================================================================
BufferDelta::BufferDelta(char* baseBuffer, char* diffBuffer, size_t numBytes)
{
#if defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)
    m_copiedDiffBuffer = NULL;
#endif // defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)

#ifndef USE_VECTOR_FOR_DATA
    m_pDiffs = NULL;
#endif

    CalculateDelta(baseBuffer, diffBuffer, numBytes);
}

//===============================================================================================
BufferDelta::BufferDelta(void)
{
#ifndef USE_VECTOR_FOR_DATA
    m_pDiffs = NULL;
#endif
#if defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)
    m_copiedDiffBuffer = NULL;
#endif // defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)
}

//===============================================================================================
BufferDelta::~BufferDelta(void)
{
    Clear();
}

void BufferDelta::Clear()
{
#if defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)

    if (m_copiedDiffBuffer != NULL)
    {
        delete [] m_copiedDiffBuffer;
        m_copiedDiffBuffer = NULL;
    }

#endif // defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)

#ifndef USE_VECTOR_FOR_DATA

    if (m_pDiffs != NULL)
    {
        delete [] m_pDiffs;
        m_pDiffs = NULL;
    }

#else
    m_data.clear();
#endif

    m_offset.clear();
    m_size.clear();
}

//===============================================================================================
void BufferDelta::CalculateDelta(const char* baseBuffer, const char* diffBuffer, const size_t numBytes)
{
    Clear();

#ifdef APPLY_DELTA

    if (baseBuffer == NULL)
    {
        // if the base buffer is NULL, then everything in the diffBuffer is considered a change, so the delta calculation is much simpler.
        m_offset.push_back(0);
        m_size.push_back(numBytes);

#ifdef USE_VECTOR_FOR_DATA
        m_data.reserve(numBytes);

        const char* pDiff = diffBuffer;

        for (size_t i = 0; i < numBytes; ++i)
        {
            m_data.push_back(*pDiff++);
        }

#else
        m_pDiffs = new char[numBytes];
        memcpy_s(m_pDiffs, numBytes, diffBuffer, numBytes);
#endif // USE_VECTOR_FOR_DATA
    }
    else
    {
#ifdef USE_VECTOR_FOR_DATA

        // loop over the entire buffer to find deltas
        for (size_t i = 0; i < numBytes; ++i)
        {
            // find a location in the buffer where the data does not match
            if (baseBuffer[i] != diffBuffer[i])
            {
                // store the current location as an offset
                size_t offset = i;
                m_offset.push_back(offset);

                // from this offset, continue incrementing i and storing the data
                // until a location is found where the data is same or the end of the buffer is reached.
                // That marks the end of the current delta, but the outer loop will continue to check for
                // additional changes.
                while (i < numBytes &&
                       baseBuffer[i] != diffBuffer[i])
                {
                    m_data.push_back(diffBuffer[i]);
                    ++i;
                }

                // now store the number of bytes that were different
                m_size.push_back(i - offset);
            }
        }

#else
        size_t totalBytes = 0;

        const char* pBase = baseBuffer;
        const char* pDiff = diffBuffer;

        //---------------------------------
        // single byte, multi delta approach

        //// first, loop over the entire buffer to find deltas
        //for (size_t i = 0; i < numBytes; ++i)
        //{
        //   // find a location in the buffer where the data does not match
        //   if (*pBase != *pDiff)
        //   {
        //      // store the current location as an offset
        //      size_t offset = i;
        //      m_offset.push_back(offset);

        //      // from this offset, continue incrementing i and storing the data
        //      // until a location is found where the data is the same or the end of the buffer is reached.
        //      // That marks the end of the current delta, but the outer loop will continue to check for
        //      // additional changes.
        //      while (i < numBytes &&
        //             *pBase != *pDiff)
        //      {
        //         ++i;
        //         ++pBase;
        //         ++pDiff;
        //      }

        //      // now store the number of bytes that were different
        //      m_size.push_back(i - offset);
        //      totalBytes += (i - offset);
        //   }

        //   ++pBase;
        //   ++pDiff;
        //}

        //// now, use size and offset information to copy data
        //m_pDiffs = new char[totalBytes];

        //size_t offset = 0;
        //size_t size = 1;
        //for (size_t i = 0; i < m_size.size(); ++i)
        //{
        //   size = m_size[i];
        //   memcpy_s(&m_pDiffs[offset], size, &diffBuffer[m_offset[i]], size);
        //   offset += size;
        //}

        //-------------------
        // multi-byte, multi delta approach

        const unsigned int* pUIBase = (const unsigned int*) pBase;
        const unsigned int* pUIDiff = (const unsigned int*) pDiff;

        // first, loop over the entire buffer to find deltas
        for (size_t i = 0; i < numBytes; ++i)
        {
            if (numBytes - i < sizeof(unsigned int))
            {
                // less than sizeof(unsigned int) bytes left, so use byte-wise comparison

                // find a location in the buffer where the data does not match
                if (baseBuffer[i] != diffBuffer[i])
                {
                    // store the current location as an offset
                    size_t offset = i;
                    m_offset.push_back(offset);

                    // from this offset, continue incrementing i and storing the data
                    // until a location is found where the data is the same or the end of the buffer is reached.
                    // That marks the end of the current delta, but the outer loop will continue to check for
                    // additional changes.
                    while (i < numBytes &&
                           baseBuffer[i] != diffBuffer[i])
                    {
                        ++i;
                    }

                    // now store the number of bytes that were different
                    m_size.push_back(i - offset);
                    totalBytes += (i - offset);
                }
            }
            else
            {
                // find a location in the buffer where the data does not match
                if (*pUIBase != *pUIDiff)
                {
                    // store the current location as an offset
                    size_t offset = i;
                    m_offset.push_back(offset);

                    // from this offset, continue incrementing i and storing the data
                    // until a location is found where the data is the same or the end of the buffer is reached.
                    // That marks the end of the current delta, but the outer loop will continue to check for
                    // additional changes.
                    while (numBytes - i >= sizeof(unsigned int) &&
                           *pUIBase != *pUIDiff)
                    {
                        i += sizeof(unsigned int);
                        ++pUIBase;
                        ++pDiff;
                    }

                    // now store the number of bytes that were different
                    m_size.push_back(i - offset);
                    totalBytes += (i - offset);
                }

                ++pUIBase;
                ++pUIDiff;
                i += sizeof(unsigned int) - 1; // one less since the loop will also increment by 1
            }
        }

        // now, use size and offset information to copy data
        m_pDiffs = new char[totalBytes];

        size_t offset = 0;
        size_t size = 1;

        for (size_t i = 0; i < m_size.size(); ++i)
        {
            size = m_size[i];
            memcpy_s(&m_pDiffs[offset], size, &diffBuffer[m_offset[i]], size);
            offset += size;
        }

        //-------------------
        // single delta approach

        //// first, find initial delta
        //size_t i = 0;
        //for (i = 0; i < numBytes; ++i)
        //{
        //   // find a location in the buffer where the data does not match
        //   if (*pBase != *pDiff)
        //   {
        //      // store the current location as an offset
        //      size_t offset = i;
        //      m_offset.push_back(offset);
        //      break;
        //  }

        //   ++pBase;
        //   ++pDiff;
        //}

        //if (i == numBytes)
        //{
        //   // there was no delta
        //}
        //else
        //{
        //   // second, find final delta (from the end of the buffer
        //   pBase = &baseBuffer[numBytes-1];
        //   pDiff = &diffBuffer[numBytes-1];

        //   for (size_t i = numBytes; i >0; --i)
        //   {
        //      // find a location in the buffer where the data does not match
        //      if (*pBase != *pDiff)
        //      {
        //         // calculate the size of the delta
        //         size_t endOffset = i;
        //         totalBytes = endOffset - m_offset[0] + 1;
        //         m_size.push_back(totalBytes);
        //         Log( logERROR, "Buffer delta starts at %d of size %d.\n", m_offset[0], totalBytes);
        //         break;
        //     }

        //      --pBase;
        //      --pDiff;
        //   }

        //   // now, use size and offset information to copy data
        //   m_pDiffs = new char[totalBytes];

        //   size_t offset = 0;
        //   size_t size = 1;
        //   for (size_t i = 0; i < m_size.size(); ++i)
        //   {
        //      size = m_size[i];
        //      memcpy_s(&m_pDiffs[offset], size, &diffBuffer[m_offset[i]], size);
        //      offset += size;
        //   }
        //}

#endif // USE_VECTOR_FOR_DATA
    }

#endif // APPLY_DELTA

#if defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)
    m_copiedDiffBufferSize = numBytes;
    m_copiedDiffBuffer = new char[m_copiedDiffBufferSize];
    memcpy(m_copiedDiffBuffer, diffBuffer, m_copiedDiffBufferSize);
#endif // defined(VERIFY_DELTA_RESULTS) || !defined(APPLY_DELTA)

}

//===============================================================================================
void BufferDelta::ApplyDelta(char* pBuffer)
{
    if (pBuffer == NULL)
    {
        return;
    }

#ifdef APPLY_DELTA
    // since the DeltaBuffer object stores all the delta data in an array,
    // this variable will help index into the array to the appropriate location.
    size_t dwDeltaDataOffset = 0;

    // for each of the recorded deltas
    for (size_t i = 0; i < m_offset.size(); i++)
    {
        // copy the stored data to the correct location of the buffer
#ifdef USE_VECTOR_FOR_DATA
        memcpy(&pBuffer[m_offset[i]], &m_data[dwDeltaDataOffset], m_size[i]);
#else
        memcpy(&pBuffer[m_offset[i]], &m_pDiffs[dwDeltaDataOffset], m_size[i]);
#endif // USE_VECTOR_FOR_DATA
        // increment the delta data to the next delta
        dwDeltaDataOffset += m_size[i];
    }

#ifdef USE_MAP_STREAMLOG
    StreamLog::Get() << "ApplyDelta" << " DataSize: " << dwDeltaDataOffset << "Num deltas: " << m_offset.size() << " \n";
#endif

#else
    memcpy(pBuffer, m_copiedDiffBuffer, m_copiedDiffBufferSize);
#endif // APPLY_DELTA

#ifdef VERIFY_DELTA_RESULTS

    for (size_t i = 0; i < m_copiedDiffBufferSize; i++)
    {
        if (pBuffer[i] != m_copiedDiffBuffer[i])
        {
            assert(!"Buffers are different!");
        }
    }

#endif //VERIFY_DELTA_RESULTS
}