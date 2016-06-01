//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains some functions shared between OCL and HSA API Trace modules
//==============================================================================

#include "APITraceUtils.h"

void DeepCopyBuffer(void** dst, const void* src, size_t size)
{
    if (NULL != dst)
    {
        if ((NULL == src) || (0 == size))
        {
            (*dst) = NULL;
        }
        else
        {
            (*dst) = new(std::nothrow) char[size]();
            memcpy(*dst, src, size);
        }
    }
}

void FreeBuffer(void* ptr)
{
    char* pCharArray = reinterpret_cast<char*>(ptr);
    delete[] pCharArray;
}
