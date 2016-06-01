//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a block of memory shared between threads and processes
//==============================================================================

#ifndef GPS_SHAREDMEMORY_H
#define GPS_SHAREDMEMORY_H

#include "misc.h"

/// forward declaration to implementation class
class SharedMemoryImpl;

/// Shared memory wrapper class
class SharedMemory
{
public:
    SharedMemory();
    ~SharedMemory();

    //=============================================================================
    /// SharedMemoryStatus enum
    /// Used as return values to report status of function calls
    //=============================================================================
    enum MemStatus
    {
        SUCCESS,
        SUCCESS_ALREADY_CREATED,  // Shared memory file has been created already
        ERROR_CREATE,             // Shared memory creation error
        ERROR_MAPPING,            // Shared memory mapping error (can't map shared memory)
        ERROR_OPEN,               // Error opening shared memory
    };

    //--------------------------------------------------------------------------
    /// Try to open an existing shared memory block or Create a new one if it
    /// doesn't exist
    /// \param bufferSize the size of the memory buffer needed
    /// \param mappingName the name that this memory buffer will be identified
    ///   by
    ///
    /// \return SharedMemoryStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    MemStatus  OpenOrCreate(int bufferSize, const char* mappingName);

    //--------------------------------------------------------------------------
    /// Open a previously created shared memory block
    /// \param mappingName the name of the memory buffer
    ///
    /// \return SharedMemoryStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    MemStatus  Open(const char* mappingName);

    //--------------------------------------------------------------------------
    /// Does this memory file exist in the system already. This does a check
    /// by trying to open the shared memory
    /// \param mappingName the name of the memory buffer
    ///
    /// \return true if it exists, false otherwise
    //--------------------------------------------------------------------------
    bool  Exists(const char* mappingName);

    //--------------------------------------------------------------------------
    /// Get a pointer to the shared memory block
    ///
    /// \return pointer to the start of memory true if it exists, otherwise
    /// return NULL
    //--------------------------------------------------------------------------
    void* Get();

    //--------------------------------------------------------------------------
    /// Close the memory file
    //--------------------------------------------------------------------------
    void  Close();

private:
    /// copy constructor made private; Prevent making copies of this object
    SharedMemory(const SharedMemory& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Prevent making copies of this object
    SharedMemory& operator= (const SharedMemory& rhs) = delete;

    /// pointer to implementation. Dependent on platform
    SharedMemoryImpl*   m_pImpl;
};

#endif // GPS_SHAREDMEMORY_H
