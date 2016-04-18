//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file APITraceUtils.h
/// \brief  This file contains some functions shared between OCL and HSA API Trace modules
//==============================================================================

#ifndef _API_TRACE_UTILS_H_
#define _API_TRACE_UTILS_H_

#include <cstdlib>
#include <cstring>
#include <new>

/// Helper template function that allocates memory and copy from source array
/// \param dst Destination
/// \param src Source
/// \param count number of array elements
template <typename T>
void DeepCopyArray(T** dst, const T* src, size_t count)
{
    if (NULL != dst)
    {
        if ((NULL == src) || (0 == count))
        {
            (*dst) = NULL;
        }
        else
        {
            (*dst) = new(std::nothrow) T[count];
            memcpy(*dst, src, sizeof(T) * count);
        }
    }
}

/// Helper template function that allocates memory and copy from source buffer
/// \param dst Destination
/// \param src Source
/// \param size Buffer size in bytes
void DeepCopyBuffer(void** dst, const void* src, size_t size);

/// Helper function to release memory allocated by DeepCopyArray
/// \param pArray The array freed
template <typename T>
void FreeArray(T* pArray)
{
    delete[] pArray;
}

/// Helper function to release memory allocated by DeepCopyBuffer
/// \param pArray The buffer freed
void FreeBuffer(void* ptr);

#endif // _API_TRACE_UTILS_H_
