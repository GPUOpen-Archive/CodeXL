//==============================================================================
// Copyright (c) 2014-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A simple memory buffer class which has functionality to detect buffer overruns
//==============================================================================

#ifndef GPS_MEMORYBUFFER_H
#define GPS_MEMORYBUFFER_H

/// A simple memory buffer class which has functionality to detect buffer overruns
class MemoryBuffer
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor
    //-----------------------------------------------------------------------------
    MemoryBuffer();

    //-----------------------------------------------------------------------------
    /// destructor
    //-----------------------------------------------------------------------------
    ~MemoryBuffer();

    //-----------------------------------------------------------------------------
    /// Allocate the memory
    /// \param bufferSize the amount of memory to allocate, in bytes
    /// \return a pointer to the allocated memory
    //-----------------------------------------------------------------------------
    unsigned char* Alloc(unsigned int bufferSize);

    //-----------------------------------------------------------------------------
    /// Free the allocated memory
    //-----------------------------------------------------------------------------
    void Free(void);

    //-----------------------------------------------------------------------------
    /// Check for a buffer overrun
    ///\ return true if buffer overrun, false if OK
    //-----------------------------------------------------------------------------
    bool BufferOverrun();

private:
    unsigned char* m_pBuffer;     ///< pointer to the buffer
    unsigned long  m_bufferSize;  ///< size of buffer, in bytess
};

#endif // GPS_MEMORYBUFFER_H
