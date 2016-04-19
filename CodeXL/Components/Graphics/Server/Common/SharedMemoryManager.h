//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface to a shared memory manager and the underlying shared memory class
//==============================================================================

#ifndef GPS_SHAREDMEMORYMANAGER_INCLUDE
#define GPS_SHAREDMEMORYMANAGER_INCLUDE

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include "defines.h"
#include "CommonTypes.h"
#if defined (_LINUX)
    #include "WinDefs.h"
#endif

// forward class definitions
class SharedMemory;
class NamedMutex;
class NamedEvent;

//=============================================================================
/// SMHeader struct
/// This structure is stored at the beginning of each shared memory and stores
/// offsets that are necessary for using the shared memory as a circular buffer
/// it also stores the current amount of data that has been put in the shared
/// memory.
//=============================================================================
struct SMHeader
{
    uint32 dwStart;       ///< offset to the start of the memory pool
    uint32 dwEnd;         ///< offset to the end of the memory pool
    uint32 dwCurrSize;    ///< current amount of data stored in the pool
    uint32 dwReadOffset;  ///< offset to the read position from the start of the pool
    uint32 dwWriteOffset; ///< offset to the write position from the start of the pool
};

//=============================================================================
/// SharedMemoryManager class
/// This class provides all the functionality for putting and getting data
/// from within the sahred memory. It also maintains any handles and pointers
/// that are associated with a shared memory
//=============================================================================
class SharedMemoryManager
{
public:
    SharedMemoryManager();
    ~SharedMemoryManager(void);

    //--------------------------------------------------------------------------
    /// Creates or opens the shared memory
    /// \param strName name of the shared memory to create
    /// \param dwMaxNumElements maximum number of elements that will be put into
    ///   the shared memory (ignored if shared memory already exists)
    /// \param dwElementSize size in bytes of a single element (ignored if shared
    ///   memory already exists)
    ///
    /// \return true if memory could be created or opened; false otherwise
    //--------------------------------------------------------------------------
    bool Create(const char* strName, unsigned long dwMaxNumElements, unsigned long dwElementSize);

    //--------------------------------------------------------------------------
    /// tries to open an existing shared memory
    ///
    /// \param strName name of the shared memory to open
    ///
    /// \return true if the shared memory could be opened; false otherwise
    //--------------------------------------------------------------------------
    bool Open(const char* strName);

    //--------------------------------------------------------------------------
    /// Closes the shared memory and releases all handles
    //--------------------------------------------------------------------------
    void Close();

    //--------------------------------------------------------------------------
    /// Puts data in the shared memory queue
    ///
    /// \param pIn pointer to data that should be put into the shared memory
    /// \param dwNumBytes number of bytes pointed to by the first parameter
    ///
    /// \return true if data could be added; false otherwise
    //--------------------------------------------------------------------------
    bool Put(void* pIn, unsigned long dwNumBytes);

    //--------------------------------------------------------------------------
    /// Gets data from the shared memory queue.
    ///
    /// \param pOut pointer to a buffer in memory where the data should be
    ///   copied into
    /// \param dwBufferSize size of the buffer in bytes
    ///
    /// \return number of bytes that were copied into the buffer; 0 if an error
    ///   occurred
    //--------------------------------------------------------------------------
    gtUInt32 Get(void* pOut, unsigned long dwBufferSize);

    //--------------------------------------------------------------------------
    /// Returns the size of the next buffer in bytes.
    ///
    /// \return number of bytes in the next buffer; 0 if there is no buffer or
    ///   if an error occurred
    //--------------------------------------------------------------------------
    gtUInt32 GetNextBufferSize();

    //--------------------------------------------------------------------------
    /// Returns the amount of data (in bytes) that is currently in the shared
    /// memory. Unless the shared memory is full, this is not the total size of
    /// the shared memory.
    ///
    /// \return number of bytes currently used in the shared memory
    //--------------------------------------------------------------------------
    gtUInt32 GetSize();

    //--------------------------------------------------------------------------
    /// Waits on and locks the mutex
    ///
    /// \param dwNumBytes number of bytes that need to be put into the memory
    /// \param dwNumBuffers the number of separate buffers that need to be put
    ///
    /// \return true if the SM is locked for writing the data; false if there was an error
    //--------------------------------------------------------------------------
    bool LockPut(unsigned long dwNumBytes, unsigned long dwNumBuffers);

    //--------------------------------------------------------------------------
    /// Releases the mutex after a call to LockPut
    //--------------------------------------------------------------------------
    void UnlockPut();

    //--------------------------------------------------------------------------
    /// Waits on and locks the mutex
    ///
    /// \return false if there was an error waiting on the mutex; true otherwise
    //--------------------------------------------------------------------------
    bool LockGet();

    //--------------------------------------------------------------------------
    /// Releases the mutex after a call to LockGet
    //--------------------------------------------------------------------------
    void UnlockGet();

    //--------------------------------------------------------------------------
    /// Resets the shared memory such that its contents will be empty
    //--------------------------------------------------------------------------
    void Reset();

    /// Copies the the command into an area of memory.
    /// \param pOut The place to store the data
    /// \param dwBufferSize The amount of data to copy
    gtUInt32 Peek(void* pOut, unsigned long dwBufferSize);

private:

    //--------------------------------------------------------------------------
    /// Tries to find a valid location to store dwNumBytes in the shared memory.
    /// \pre LockPut has been called successfully
    /// \param dwNumBytes the number of bytes that need to be put into the
    /// shared memory
    /// \param rpPutLocation
    /// \param rdwChunkSize
    /// \return true if a large enough location is found; false otherwise
    //--------------------------------------------------------------------------
    bool FindPutLocation(unsigned long dwNumBytes, void*& rpPutLocation, unsigned long& rdwChunkSize);

    //--------------------------------------------------------------------------
    /// Returns the memory address from which the next Get should be performed.
    /// \pre LockGet has been called successfully
    /// \return NULL if there is no data to read; a valid address otherwise.
    //--------------------------------------------------------------------------
    void* FindGetLocation();

private:
    SharedMemory* m_pMapFile;          ///< Shared memory wrapper
    NamedMutex*   m_pSMMutex;          ///< the mutex to the mapped file
    NamedMutex*   m_pReadMutex;        ///< the mutex for reading
    NamedMutex*   m_pWriteMutex;       ///< the mutex for writing
    NamedEvent*   m_pChunkRead;        ///< Event to signal reading is not occuring
    NamedEvent*   m_pChunkWritten;     ///< Event to signal writing is not occuring
    SMHeader*     m_pHeader;           ///< pointer to header struct in the shared memory
    char* m_pPool;                     ///< pointer to pool within the shared memory
    char  m_strName[ PS_MAX_PATH ];    ///< name of the shared memory
};

//=============================================================================
//    Exported functions from SharedMemory.dll
//=============================================================================

//-----------------------------------------------------------------------------
/// Checks to see if the specified shared memory already exists
///
/// \param strName name of the shared memory to look for
///
/// \return true if the mutex and shared memory exists; false otherwise
//-----------------------------------------------------------------------------
bool smExists(const char* strName);

//-----------------------------------------------------------------------------
/// Creates a named shared memory if it doesn't already exist; opens the memory
/// if it does exist
///
/// \param strName name of the shared memory to create
/// \param dwMaxNumElements max number of elements that can fit in the memory
/// \param dwElementSize size of a single element in bytes
///
/// \return true if the shared memory could be created or opened; false otherwise
//-----------------------------------------------------------------------------
bool smCreate(const char* strName, unsigned long dwMaxNumElements, unsigned long dwElementSize);

//-----------------------------------------------------------------------------
/// Resets the named shared memory
///
/// \param strName name of the shared memory to reset
//-----------------------------------------------------------------------------
void smReset(const char* strName);

//-----------------------------------------------------------------------------
/// Closes the named shared memory
///
/// \param strName name of the shared memory to close
//-----------------------------------------------------------------------------
void smClose(const char* strName);

//-----------------------------------------------------------------------------
/// Opens the named shared memory for this process. Does not lock the shared memory.
///
/// \param strName name of the shared memory to open
///
/// \return true if the shared memory could be opened; false otherwise
//-----------------------------------------------------------------------------
bool smOpen(const char* strName);

//-----------------------------------------------------------------------------
/// Puts data into the shared memory for access in a FIFO manner
///
/// \param strName name of the shared memory to put data in
/// \param pIn pointer to the data that should be put in the shared memory
/// \param dwNumBytes number of bytes of data pointed to by the previous parameter
///
/// \return true if the data could be put in the shared memory; false otherwise
//-----------------------------------------------------------------------------
bool smPut(const char* strName, void* pIn, unsigned long dwNumBytes);

//=============================================================================
/// Copies the next buffer in the named shared memory to the specified location.
/// If either of the last two parameters are NULL (or 0), this function will
/// return the size of the next buffer without removing it from the shared memory
///
/// \param strName name of the shared memory to get data from
/// \param out buffer that the data in the shared memory should be copied into
/// \param dwNumBytes maximum number of bytes to copy into the previous buffer
///
/// \return number of bytes that were copied into the buffer; 0 if no data was available
//=============================================================================
gtUInt32 smGet(const char* strName, void* pOut, unsigned long dwBufferSize);

//=============================================================================
/// Copies the next buffer in the named shared memory to the specified location.
/// If either of the last two parameters are NULL (or 0), this function will
/// return the size of the next buffer without removing it from the shared memory
///
/// \param strName name of the shared memory to get data from
/// \param out buffer that the data in the shared memory should be copied into
/// \param dwNumBytes maximum number of bytes to copy into the previous buffer
///
/// \return number of bytes that were copied into the buffer; 0 if no data was available
//=============================================================================
gtUInt32 smPeek(const char* strName, void* out, unsigned long dwNumBytes);

//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \param strName name of the shared memory to lock
/// \param dwNumBytes number of bytes that need to be put into the memory
/// \param dwNumBuffers the number of separate buffers that need to be put
///
/// \return pointer to a location where the data can be copied into; NULL if
///   there is not enough room for the requested number of bytes or if there
///   was an error waiting on the mutex
//--------------------------------------------------------------------------
bool smLockPut(const char* strName, unsigned long dwNumBytes, unsigned long dwNumBuffers);

//--------------------------------------------------------------------------
/// Releases the mutex after a call to smLockPut
///
/// \param strName name of the shared memory to unlock
//--------------------------------------------------------------------------
void smUnlockPut(const char* strName);

//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \param strName name of the shared memory to lock
///
/// \return pointer to the location of the next buffer; NULL if there is no
///   data to read or if there was an error waiting on the mutex
//--------------------------------------------------------------------------
bool smLockGet(const char* strName);

//--------------------------------------------------------------------------
/// Releases the mutex after a call to smLockGet
///
/// \param strName name of the shared memory to unlock
//--------------------------------------------------------------------------
void smUnlockGet(const char* strName);


#endif //GPS_SHAREDMEMORYMANAGER_INCLUDE

