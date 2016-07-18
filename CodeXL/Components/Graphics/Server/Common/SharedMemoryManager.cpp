//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface to a shared memory manager and the underlying
///         shared memory class
//==============================================================================

#include <stdio.h>
#if defined (_WIN32)
    #include <windows.h>
#endif

#include <algorithm>

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <map>
#include "Logger.h"
#include "misc.h"

#include "NamedMutex.h"
#include "NamedEvent.h"
#include "SharedMemory.h"

#include "SharedMemoryManager.h"

/// Definition
#define MOVEPTR( ptr, bytes ) (ptr) = &((char*)(ptr))[(bytes)]

/// Definition
#define DWORD_SIZE sizeof( gtUInt32 )

/// this defines the header size for a single buffer that is in the shared memory
/// the first DWORD is the total size of the buffer (may be larger than the actual shared memory)
/// the second DWORD is the size of the chunk that was written in shared memory
///    ( must be <= the first DWORD, and is always smaller than the size of the shared memory)
#define BUFFER_HEADER_SIZE (DWORD_SIZE * 2)

static std::map< gtASCIIString, SharedMemoryManager* >* g_sharedMemoryMap = NULL; ///< Shared memory map

/// Named mutex for shared memory map; one per process
static NamedMutex*  g_MapMutex = NULL;

// define this value to the name of the shared memory that you want to debug
//#define DEBUG_SHARED_MEM "PLUGINS_TO_GPS"

//-----------------------------------------------------------------------------
/// Only available internal to this file; used to create a mutex to ensure that
/// the shared memory map is not accessed by two threads at the same time. This
/// is called at the beginning of each call to smOpen and smCreate
//-----------------------------------------------------------------------------
bool InitSM()
{
    // just return if map mutex has already been created
    if (g_MapMutex != NULL)
    {
        return true;
    }

    Log(logMESSAGE, "Initializing SharedMemory library\n");

    // create shared memory map
    g_sharedMemoryMap = new std::map< gtASCIIString, SharedMemoryManager* >();

    // try to open the existing map mutex
    g_MapMutex = new NamedMutex();

    if (false == g_MapMutex->Open("GPS_SharedMemoryMapMutex"))
    {
        // if the mutex wasn't opened
        // then try to create it
        if (false == g_MapMutex->OpenOrCreate("GPS_SharedMemoryMapMutex"))
        {
            Log(logERROR, "Failed to Initialize SharedMemory - mutex creation failed: %d\n", osGetLastSystemError());
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Only available internal to this file; used to close the handle to the sm
/// map mutex when no more shared memories are opened. This is called at tne
/// end of each call to smClose
//-----------------------------------------------------------------------------
void DeinitSM()
{
    if (g_sharedMemoryMap->size() == 0)
    {
        // close the handle to the mutex if there are no more shared memories to manage
        delete g_MapMutex;
        g_MapMutex = NULL;

        delete g_sharedMemoryMap;
        g_sharedMemoryMap = NULL;
    }
}

//-----------------------------------------------------------------------------
/// This function is only available internal to this file and is used to get
/// a pointer to a SharedMemory class from the shared memory manager (map).
/// Error messages are printed for all unexpected cases.
/// \param strName Name of the shared memory to get
/// \return Pointer to the shared memory
//-----------------------------------------------------------------------------
SharedMemoryManager* GetSM(const char* strName)
{
    if (strName == NULL)
    {
        Log(logERROR, "Cannot access shared memory because a name was not provided.\n");
        return NULL;
    }

    gtASCIIString strKey(strName);
    SharedMemoryManager* pSM = NULL;

    if (false == g_MapMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return NULL;
    }

    std::map< gtASCIIString, SharedMemoryManager* >::const_iterator iterFind = g_sharedMemoryMap->find(strKey);

    if (iterFind != g_sharedMemoryMap->end())
    {
        pSM = iterFind->second;
    }

    g_MapMutex->Unlock();

    return pSM;
}

//=============================================================================
//    Exported functions from SharedMemory.dll
//=============================================================================

//=============================================================================
/// Checks to see if the specified shared memory already exists
///
/// \param strName name of the shared memory to look for
///
/// \return true if the mutex and shared memory exists; false otherwise
//=============================================================================
bool smExists(const char* strName)
{
    if (strName == NULL)
    {
        return false;
    }

    char strMutex[ PS_MAX_PATH ];
    sprintf_s(strMutex, PS_MAX_PATH, "%s_mutex", strName);

    // first try to open the mutex
    NamedMutex checkMutex;

    if (false == checkMutex.Open(strMutex))
    {
        return false;
    }

    // then try to open the shared memory
    SharedMemory mapFile;

    if (false == mapFile.Exists(strName))
    {
        return false;
    }

    return true;
}

//=============================================================================
/// Creates a named shared memory if it doesn't already exist; opens the memory
/// if it does exist
///
/// \param strName name of the shared memory to create
/// \param dwMaxNumElements max number of elements that can fit in the memory
/// \param dwElementSize size of a single element in bytes
///
/// \return true if the shared memory could be created or opened; false otherwise
//=============================================================================
bool smCreate(const char* strName, unsigned long dwMaxNumElements, unsigned long dwElementSize)
{
    PsAssert(strName != NULL);
    PsAssert(dwMaxNumElements > 0);
    PsAssert(dwElementSize > 0);

    if (InitSM() == false)
    {
        return false;
    }

    if (false == g_MapMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return false;
    }

    gtASCIIString strKey(strName);

    // check to see if shared memory exists already in this process. If so, it doesn't need to be
    // created again
    std::map< gtASCIIString, SharedMemoryManager* >::const_iterator iterExists = g_sharedMemoryMap->find(strKey);

    if (iterExists != g_sharedMemoryMap->end())
    {
        g_MapMutex->Unlock();
        return true;
    }

    SharedMemoryManager* pSM = NULL;

    try
    {
        pSM = new SharedMemoryManager();
    }
    catch (std::bad_alloc)
    {
        Log(logERROR, "out of memory: could not create a new shared memory\n");
        g_MapMutex->Unlock();
        return false;
    }

    // try to create the memory
    if (pSM->Create(strName, dwMaxNumElements, dwElementSize) == false)
    {
        // could not create it
        Log(logERROR, "smCreate( %s, %lu, %lu ) failed because of error: %d\n", strName, dwMaxNumElements, dwElementSize, osGetLastSystemError());
        g_MapMutex->Unlock();
        SAFE_DELETE(pSM);
        return false;
    }

    (*g_sharedMemoryMap)[ strKey ] = pSM;

    g_MapMutex->Unlock();

    return true;
}

//=============================================================================
/// Opens the named shared memory
///
/// \param strName name of the shared memory to open
///
/// \return true if the shared memory could be opened; false otherwise
//=============================================================================
bool smOpen(const char* strName)
{
    PsAssert(strName != NULL);

    if (InitSM() == false)
    {
        return false;
    }

    // see if the shared memory is already open
    if (GetSM(strName) != NULL)
    {
        return true;
    }

    if (false == g_MapMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return false;
    }

    SharedMemoryManager* pSM = NULL;

    try
    {
        pSM = new SharedMemoryManager();
    }
    catch (std::bad_alloc)
    {
        Log(logERROR, "out of memory: could not create a new shared memory\n");
        g_MapMutex->Unlock();
        return false;
    }

    // try to open the memory
    if (pSM->Open(strName) == false)
    {
        // could not open it
        Log(logERROR, "smOpen failed because \"%s\" is not the name of created shared memory.\n", strName);
        g_MapMutex->Unlock();
        SAFE_DELETE(pSM);
        return false;
    }

    gtASCIIString strKey(strName);
    (*g_sharedMemoryMap)[ strKey ] = pSM;

    g_MapMutex->Unlock();

    return true;
}

//-----------------------------------------------------------------------------
/// Resets the named shared memory
///
/// \param strName name of the shared memory to reset
//-----------------------------------------------------------------------------
void smReset(const char* strName)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return;
    }

    pSM->Reset();
}


//=============================================================================
/// Closes the named shared memory
///
/// \param strName name of the shared memory to close
//=============================================================================
void smClose(const char* strName)
{
    // Abort if the mutex is NULL. This can happen if smClose is being
    // called after a call to smCreate or smOpen has failed.
    if (g_MapMutex == NULL)
    {
        Log(logERROR, "Error occurred when closing shared memory\n");
        //      LogConsole(logERROR, "g_MapMutex is NULL and has been deleted. There are %d elements in g_sharedMemoryMap\n", g_sharedMemoryMap.size());
        return;
    }

    if (false == g_MapMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return;
    }

    gtASCIIString strKey(strName);
    std::map< gtASCIIString, SharedMemoryManager* >::iterator iterClose = g_sharedMemoryMap->find(strKey);

    if (iterClose != g_sharedMemoryMap->end())
    {
        // this shared memory name exists, so close it
        iterClose->second->Close();

        // free the memory
        delete iterClose->second;

        // remove the shared memory from the map
        g_sharedMemoryMap->erase(iterClose);
    }

    g_MapMutex->Unlock();

    DeinitSM();
}

//=============================================================================
/// Puts data into the shared memory for access in a FIFO manner
///
/// \param strName name of the shared memory to put data in
/// \param pIn pointer to the data that should be put in the shared memory
/// \param dwNumBytes number of bytes of data pointed to by the previous parameter
///
/// \return true if the data could be put in the shared memory; false otherwise
//=============================================================================
bool smPut(const char* strName, void* pIn, unsigned long dwNumBytes)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return false;
    }

    return pSM->Put(pIn, dwNumBytes);
}

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
gtUInt32 smGet(const char* strName, void* out, unsigned long dwNumBytes)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return 0;
    }

    if (out == NULL || dwNumBytes == 0)
    {
        return pSM->GetNextBufferSize();
    }

    return pSM->Get(out, dwNumBytes);
}

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
gtUInt32 smPeek(const char* strName, void* out, unsigned long dwNumBytes)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return 0;
    }

    if (out == NULL || dwNumBytes == 0)
    {
        return pSM->GetNextBufferSize();
    }

    return pSM->Peek(out, dwNumBytes);
}

//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \param strName name of the shared memory to lock
/// \param dwNumBytes number of bytes that need to be put into the memory
/// \param dwNumBuffers the number of separate buffers that need to be put
/// \return pointer to a location where the data can be copied into; NULL if
///   there is not enough room for the requested number of bytes or if there
///   was an error waiting on the mutex
//--------------------------------------------------------------------------
bool smLockPut(const char* strName, unsigned long dwNumBytes, unsigned long dwNumBuffers)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return false;
    }

    return pSM->LockPut(dwNumBytes, dwNumBuffers);
}

//--------------------------------------------------------------------------
/// Releases the mutex after a call to smLockPut
///
/// \param strName name of the shared memory to unlock
//--------------------------------------------------------------------------
void smUnlockPut(const char* strName)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return;
    }

    pSM->UnlockPut();
}

//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \param strName name of the shared memory to lock
///
/// \return pointer to the location of the next buffer; NULL if there is no
///   data to read or if there was an error waiting on the mutex
//--------------------------------------------------------------------------
bool smLockGet(const char* strName)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return false;
    }

    return pSM->LockGet();
}

//--------------------------------------------------------------------------
/// Releases the mutex after a call to smLockGet
///
/// \param strName name of the shared memory to unlock
//--------------------------------------------------------------------------
void smUnlockGet(const char* strName)
{
    PsAssert(strName != NULL);
    SharedMemoryManager* pSM = GetSM(strName);

    PsAssert(pSM != NULL);

    if (pSM == NULL)
    {
        Log(logERROR, "%s failed because '%s' is not the name of an opened shared memory.\n", __FUNCTION__, strName);
        return;
    }

    pSM->UnlockGet();
}



//=============================================================================
///
///                SharedMemoryManager class Definition
///
//=============================================================================
/// SharedMemory class
/// This class provides all the functionality for putting and getting data
/// from within the sahred memory. It also maintains any handles and pointers
/// that are associated with a shared memory
//=============================================================================
SharedMemoryManager::SharedMemoryManager()
    : m_pHeader(NULL),
      m_pPool(NULL)
{
    memset(m_strName, 0, PS_MAX_PATH);
    m_pMapFile = new SharedMemory();
    m_pSMMutex = new NamedMutex();
    m_pReadMutex = new NamedMutex();
    m_pWriteMutex = new NamedMutex();
    m_pChunkRead = new NamedEvent();
    m_pChunkWritten = new NamedEvent();
}

SharedMemoryManager::~SharedMemoryManager()
{
    Close();
    delete m_pMapFile;
    delete m_pSMMutex;
    delete m_pReadMutex;
    delete m_pWriteMutex;
    delete m_pChunkRead;
    delete m_pChunkWritten;
}

//=============================================================================
///         Public Member Functions
//=============================================================================
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
bool SharedMemoryManager::Create(const char* strName, unsigned long dwMaxNumElements, unsigned long dwElementSize)
{
    sprintf_s(m_strName, PS_MAX_PATH, "%s", strName);

    // Several things are needed to support the shared memory
    // 1) Read Mutex
    //    This allows only 1 process to read from the shared memory at a single time
    //    Note that a read and a write can happen simultaneously in a synchronized manner
    // 2) Write Mutex
    //    This allows only 1 process to write to the shared memory at a single time
    //    Note that a read and a write can happen simultaneously in a synchronized manner
    // 3) Chunk Read Event
    //    Used to synchronize simultaneous read / write
    //    Signals to a writing process that a read has completed / is not in progess
    // 4) Chunk Written Event
    //    Used to synchronize simultaneous read / write
    //    Signals to a reading process that a write has completed / is not in progess
    // 5) File Mapping
    //    This is the actual location where the data in the shared memory is stored
    //    Without a "View of the Mapped File", it is useless
    // 6) View of the Mapped File
    //    This gives our process a pointer to the File Mapping so that we can put data in it
    //    Note: We place a header (m_pHeader) at the start of this view to keep track of read/write offsets and the current size
    //          In order for this to work properly across processes, offsets from the start of the view are used in the header
    //          instead of pointers. Since each process has it's own view of the memory, the addresses for one process are
    //          different from addresses in the other process, so pointers are invalid. Instead, the pointers / addresses are
    //          calculated as offsets from the View of the Mapped File.

    char strTmp[ PS_MAX_PATH ];

    // create the SM Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_mutex", m_strName);

    // this will create the mutex
    // or open it if it was already created
    if (false == m_pSMMutex->OpenOrCreate(strTmp, true))
    {
        Log(logERROR, "Failed to create sm mutex: %d\n", osGetLastSystemError());
        return false;
    }

    // create the Read Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_read_mutex", m_strName);

    // this will create the mutex
    // or open it if it was already created
    if (false == m_pReadMutex->OpenOrCreate(strTmp, true))
    {
        Log(logERROR, "Failed to create read mutex: %d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        return false;
    }

    // create the Write Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_write_mutex", m_strName);

    // this will create the mutex
    // or open it if it was already created
    if (false == m_pWriteMutex->OpenOrCreate(strTmp, true))
    {
        Log(logERROR, "Failed to create write mutex: %d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        Close();
        return false;
    }

    // Create the Chunk Read Event
    // create it with initial state of TRUE so that we are allowed to write into it
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_chunk_read", m_strName);

    if (false == m_pChunkRead->Create(strTmp, true))
    {
        Log(logERROR, "Failed to create %s Event. Error %lu\n", strTmp, osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    // Create the Chunk Write Event
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_chunk_written", m_strName);

    if (false == m_pChunkWritten->Create(strTmp))
    {
        Log(logERROR, "Failed to create %s Event. Error %lu\n", strTmp, osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    // create File Mapping
    gtUInt32 dwTotalSize = sizeof(SMHeader) +
                           (dwMaxNumElements * dwElementSize) +
                           (dwMaxNumElements * BUFFER_HEADER_SIZE);

    //   Log(logMESSAGE, "Calling SharedMemory::Create '%s', %d\n", m_strName, dwTotalSize);

    SharedMemory::MemStatus  status = m_pMapFile->OpenOrCreate(dwTotalSize, m_strName);

    if (SharedMemory::ERROR_CREATE == status)
    {
        Log(logERROR, "Can't CreateFileMapping for %s!\n", m_strName);
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    // Get a pointer to the file-mapped shared memory
    if (SharedMemory::ERROR_MAPPING == status)
    {
        Log(logERROR, "Can't MapViewOfFile for %s!\n", m_strName);
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    bool bInitMem = true;

    if (SharedMemory::SUCCESS_ALREADY_CREATED == status)
    {
        bInitMem = false;
    }

    // Get a pointer to the file-mapped shared memory
    void* pMemory = m_pMapFile->Get();

    // Initialize memory if this is the first process
    if (bInitMem == true)
    {
        //      Log(logMESSAGE, "InitMem called\n");

        // write header at top of shared memory
        SMHeader smHeader;
        smHeader.dwCurrSize = 0;
        smHeader.dwStart = sizeof(SMHeader);   // beginning is offset by the header size
        smHeader.dwEnd = dwTotalSize;
        smHeader.dwReadOffset = 0;
        smHeader.dwWriteOffset = 0;

        memcpy_s(pMemory, dwTotalSize, &smHeader, sizeof(SMHeader));
    }

    // make header point to start of shared memory
    m_pHeader = (SMHeader*) pMemory;
    PsAssert(m_pHeader != NULL);
    m_pPool = (char*) pMemory + m_pHeader->dwStart;
    PsAssert(m_pPool != NULL);

    // clear the shared memory
    Reset();

#ifdef DEBUG_SHARED_MEM

    if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
    {
        Log(logMESSAGE, "--------> SM max size     is %lu (Create)\n", m_pHeader->dwEnd - m_pHeader->dwStart);
        Log(logMESSAGE, "--------> SM current size is %lu (Create)\n", m_pHeader->dwCurrSize);
    }

#endif

    // release the read / write mutexes so that processes can start using them
    m_pSMMutex->Unlock();
    m_pReadMutex->Unlock();
    m_pWriteMutex->Unlock();

    Log(logMESSAGE, "Created SharedMemory: \"%s\"\n", m_strName);

    return true;
}

void SharedMemoryManager::Reset()
{
    // Get the mutex so that we're sure we are the only ones
    // accessing the shared memory
    if (false == m_pSMMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
    }

    m_pHeader->dwCurrSize = 0;
    m_pHeader->dwReadOffset = 0;
    m_pHeader->dwWriteOffset = 0;

    memset(m_pPool, '\0', m_pHeader->dwEnd - m_pHeader->dwStart);

    m_pSMMutex->Unlock();
}

//--------------------------------------------------------------------------
/// tries to open an existing shared memory
///
/// \param strName name of the shared memory to open
///
/// \return true if the shared memory could be opened; false otherwise
//--------------------------------------------------------------------------
bool SharedMemoryManager::Open(const char* strName)
{
    sprintf_s(m_strName, PS_MAX_PATH, "%s", strName);

    char strTmp[ PS_MAX_PATH ];

    // Open Read Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_mutex", m_strName);

    if (false == m_pSMMutex->Open(strTmp, true))
    {
        Log(logERROR, "Failed to open sm mutex: %d\n", osGetLastSystemError());
        return false;
    }

    // Get the mutex so that we're sure we are the only ones
    // accessing the shared memory
    if (false == m_pSMMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return false;
    }

    // Open Read Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_read_mutex", m_strName);

    if (false == m_pReadMutex->Open(strTmp, true))
    {
        Log(logERROR, "Failed to open read mutex: %d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        return false;
    }

    // Get the mutex so that we're sure we are the only ones
    // accessing the shared memory
    if (false == m_pReadMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        return false;
    }

    // Open Write Mutex
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_write_mutex", m_strName);

    if (false == m_pWriteMutex->Open(strTmp, true))
    {
        Log(logERROR, "Failed to open write mutex: %d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        Close();
        return false;
    }

    // Get the mutex so that we're sure we are the only ones
    // accessing the shared memory
    if (false == m_pWriteMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        Close();
        return false;
    }


    // Open the Chunk Read Event
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_chunk_read", m_strName);

    if (false == m_pChunkRead->Open(strTmp, true))
    {
        Log(logERROR, "Failed to open %s Event. Error %lu\n", strTmp, osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    // Open the Chunk Write Event
    memset(strTmp, 0, PS_MAX_PATH);
    sprintf_s(strTmp, PS_MAX_PATH, "%s_chunk_written", m_strName);

    if (false == m_pChunkWritten->Open(strTmp, true))
    {
        Log(logERROR, "Failed to open %s Event. Error %lu\n", strTmp, osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    //   Log(logMESSAGE, "Calling SharedMemory::Open '%s'\n", m_strName);

    // open shared mem
    SharedMemory::MemStatus  status = m_pMapFile->Open(m_strName);

    if (SharedMemory::ERROR_OPEN == status)
    {
        Log(logERROR, "Can't OpenFileMapping for %s!\n", m_strName);
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    if (SharedMemory::ERROR_MAPPING == status)
    {
        Log(logERROR, "Can't MapViewOfFile for opening %s; error %d\n", m_strName, osGetLastSystemError());
        m_pSMMutex->Unlock();
        m_pReadMutex->Unlock();
        m_pWriteMutex->Unlock();
        Close();
        return false;
    }

    void* pMemory = m_pMapFile->Get();

    // make header point to start of shared memory
    m_pHeader = (SMHeader*) pMemory;
    PsAssert(m_pHeader != NULL);
    m_pPool = (char*) pMemory + m_pHeader->dwStart;
    PsAssert(m_pPool != NULL);

    //   Log(logMESSAGE, "Opened Shared Memory - %s\n", m_strName);

    m_pSMMutex->Unlock();
    m_pReadMutex->Unlock();
    m_pWriteMutex->Unlock();

    return true;
}

//--------------------------------------------------------------------------
/// Closes the shared memory and releases all handles
//--------------------------------------------------------------------------
void SharedMemoryManager::Close()
{
    m_pSMMutex->Close();
    m_pReadMutex->Close();
    m_pWriteMutex->Close();
    m_pChunkRead->Close();
    m_pChunkWritten->Close();
    m_pMapFile->Close();
    m_pPool = NULL;
    m_pHeader = NULL;
}

//--------------------------------------------------------------------------
/// Puts data in the shared memory queue
///
/// \param pIn pointer to data that should be put into the shared memory
/// \param dwNumBytes number of bytes pointed to by the first parameter
///
/// \return true if data could be added; false otherwise
//--------------------------------------------------------------------------
bool SharedMemoryManager::Put(void* pIn, unsigned long dwNumBytes)
{
#ifdef DEBUG_SHARED_MEM

    if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
    {
        Log(logMESSAGE, "Put Response Size = %lu\n", dwNumBytes);
    }

#endif //DEBUG_SHARED_MEM

    if (pIn == NULL || dwNumBytes == 0)
    {
        return false;
    }

    void* pPtr = NULL;      //<-- points to location in shared memory that we are writing to
    void* pOrig = pIn;      //<-- increments along input buffer as chunks are written into SM
    unsigned long dwChunkSize = 0;

    unsigned long dwBytesWritten = 0;

    while (dwBytesWritten < dwNumBytes)
    {
        // Make sure we are allowed to write
        if (false == m_pChunkRead->Wait())
        {
            Log(logERROR, "Error occurred while waiting for chunk written. Error %lu\n", osGetLastSystemError());
            return false;
        }

        if (false == m_pSMMutex->Lock())
        {
            Log(logERROR, "Error occurred while waiting for sm mutex. Error %lu\n", osGetLastSystemError());
            return false;
        }

        if (FindPutLocation(dwNumBytes - dwBytesWritten, pPtr, dwChunkSize) == false)
        {
            // this just continues, because it should be able to find a put location after more data has been read
            m_pSMMutex->Unlock();
            continue;
        }

#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "Put Chunk Size = %lu for %lu / %lu\n", dwChunkSize, dwBytesWritten + dwChunkSize, dwNumBytes);
        }

#endif //DEBUG_SHARED_MEM
        // write total buffer size
        memcpy_s(pPtr, DWORD_SIZE, &dwNumBytes, DWORD_SIZE);
        MOVEPTR(pPtr, DWORD_SIZE);

        // write chunk size
        memcpy_s(pPtr, DWORD_SIZE, &dwChunkSize, DWORD_SIZE);
        MOVEPTR(pPtr, DWORD_SIZE);

        // copy chunk of original input data into SM
        memcpy_s(pPtr, dwChunkSize, pOrig, dwChunkSize);
        MOVEPTR(pOrig, dwChunkSize);

        // increment the number of written bytes by the size of the chunk we just wrote
        dwBytesWritten += dwChunkSize;

        if (false == m_pChunkWritten->Signal())
        {
            // return true since we were able to put the data in, we just weren't able to signal it to be read
            Log(logERROR, "SetEvent on chunk_written failed. Error %lu\n", osGetLastSystemError());
        }

        // if there is still data to write, reset the chunkRead event so that reading the data can happen
        // note we could also make sure that there isn't enough room for the rest of the data, this would allow us to
        // 'wrap around' if needed and continue writing
        if (m_pHeader->dwEnd - m_pHeader->dwStart - m_pHeader->dwCurrSize < dwNumBytes - dwBytesWritten + BUFFER_HEADER_SIZE)   //dwBytesWritten < dwNumBytes )
        {
            m_pChunkRead->Reset();
        }

        // do some housekeeping on the header values
        m_pHeader->dwCurrSize += (dwChunkSize + BUFFER_HEADER_SIZE);
        m_pHeader->dwWriteOffset += (dwChunkSize + BUFFER_HEADER_SIZE);

        if (m_pHeader->dwWriteOffset >= m_pHeader->dwEnd - m_pHeader->dwStart)
        {
            m_pHeader->dwWriteOffset = 0;
        }

#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "put-----> SM current size is %lu \t(+%lu)\n", m_pHeader->dwCurrSize, dwChunkSize + BUFFER_HEADER_SIZE);
            Log(logMESSAGE, "put-----> SM read offset  is %lu \n", m_pHeader->dwReadOffset);
            Log(logMESSAGE, "put-----> SM write offset is %lu \n", m_pHeader->dwWriteOffset);
        }

#endif

        m_pSMMutex->Unlock();
    }

    return true;
}

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
gtUInt32 SharedMemoryManager::Get(void* pOut, unsigned long dwBufferSize)
{
    if (pOut == NULL ||
        dwBufferSize == 0)
    {
        return 0;
    }

    void* pCopy = pOut;     //<-- this pointer will increment along the buffer as chunks are copied
    gtUInt32 dwBytesRead = 0;

    bool bFirstPass = true;

    // the expected total size is written at the top of each chunk, followed by the size of the chunk.
    // we'll use the first "total size" value as the expected total size of the entire buffer
    gtUInt32 dwExpectedTotalSize = 0;

    while (dwBytesRead < dwExpectedTotalSize || bFirstPass)
    {
        // wait for a chunk to be written
        if (false == m_pChunkWritten->Wait())
        {
            Log(logERROR, "Error occurred while waiting for chunk written:%d\n", osGetLastSystemError());
            return dwBytesRead;
        }

        if (false == m_pSMMutex->Lock())
        {
            Log(logERROR, "Error occurred while waiting for sm mutex. Error %lu\n", osGetLastSystemError());
            m_pChunkWritten->Reset();
            return dwBytesRead;
        }

        char* ptr = (char*) FindGetLocation();

        if (ptr == NULL)
        {
            // since chunkWritten was signaled, there should definitely be data to get
            Log(logERROR, "Unable to find get location. Error %lu\n", osGetLastSystemError());
            m_pChunkWritten->Reset();
            m_pSMMutex->Unlock();
            return dwBytesRead;
        }

        if (bFirstPass)
        {
            dwExpectedTotalSize = ((gtUInt32*) ptr)[ 0 ];

            // if the expected total size is greater than the supplied buffer, log an error and return.
            // since this means there was a problem with our system, I'm purposefully not filling the buffer with "as much data as we could"
            if (dwExpectedTotalSize > dwBufferSize)
            {
                Log(logERROR, "First pass: buffer (%lu bytes) not large enough to hold next message (%lu bytes).\n", dwBufferSize, dwExpectedTotalSize);
                m_pChunkWritten->Reset();
                m_pSMMutex->Unlock();
                return 0;
            }

            bFirstPass = false;
        }

        // the first value in the total size of the message
        // this should be the same as when we read it the first time (dwExpectedTotalSize)
        gtUInt32 dwTotalBufferSize = ((gtUInt32*) ptr)[ 0 ];

        // store the expected total size as a check to make sure we are still reading for the same response
        if (dwTotalBufferSize != dwExpectedTotalSize)
        {
            // log the error and break, so that the already read data is returned and we don't interfere with the next response
            Log(logERROR, "Response reading for buffer of size %lu started reading for another buffer of size %lu\n", dwExpectedTotalSize, dwTotalBufferSize);
            m_pChunkWritten->Reset();
            m_pSMMutex->Unlock();
            break;
        }

        // the buffer size is valid compared to the expected size
        // so now we can move past that data.
        MOVEPTR(ptr, DWORD_SIZE);

        // read the size of the next chunk and add it to the number of bytes read
        // this better be less then the total size of the buffer
        gtUInt32 dwChunkSize = ((gtUInt32*) ptr)[ 0 ];

        // NOTE: if this happens, it is a very bad situation, means something else is wrong in the system
        // the steps here are to try to correct it.
        if (dwBytesRead + dwChunkSize > dwExpectedTotalSize)
        {
            Log(logASSERT, "Num bytes read (%lu) > Expected size (%lu)\n", dwBytesRead + dwChunkSize, dwExpectedTotalSize);
            PsAssert(!(dwBytesRead + dwChunkSize > dwExpectedTotalSize));
            dwChunkSize = std::min(dwExpectedTotalSize - dwBytesRead, dwChunkSize);
        }

        // move past the chunk size
        MOVEPTR(ptr, DWORD_SIZE);

        // copy the data from the shared memory into the input buffer,
        // copy into the current location based on chunk sizes
        memcpy_s(pCopy, dwBufferSize, ptr, dwChunkSize);

        dwBytesRead += dwChunkSize;

        // move the copy pointer down the buffer
        MOVEPTR(pCopy, dwChunkSize);

        // do housekeeping on the header data
        m_pHeader->dwCurrSize -= (dwChunkSize + BUFFER_HEADER_SIZE);
        m_pHeader->dwReadOffset += (dwChunkSize + BUFFER_HEADER_SIZE);

        // check for a wrapped ReadOffset
        if (m_pHeader->dwReadOffset >= m_pHeader->dwEnd - m_pHeader->dwStart)
        {
            m_pHeader->dwReadOffset = 0;
        }

#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "get-----> SM current size is %lu\t(-%lu)\n", m_pHeader->dwCurrSize, (dwChunkSize + BUFFER_HEADER_SIZE));
            Log(logMESSAGE, "get-----> SM read offset  is %lu\n", m_pHeader->dwReadOffset);
            Log(logMESSAGE, "get-----> SM write offset is %lu\n", m_pHeader->dwWriteOffset);
        }

#endif

        if (m_pHeader->dwCurrSize == 0)
        {
            // in this case, we could also reset the header information back to the beginning to
            // reduce the cases where we have to "wrap around"
            m_pChunkWritten->Reset();
            //#ifdef DEBUG_SHARED_MEM
            //if ( strcmp(m_strName, DEBUG_SHARED_MEM) == 0 )
            //{
            //   Log( logMESSAGE, "get-----> Reset chunk_written\n" );
            //}
            //#endif
        }

        // set that we've read the chunk, so another can be written if needed
        if (false == m_pChunkRead->Signal())
        {
            // return true since we were able to get the data, we just weren't able to signal more to be written
            Log(logERROR, "SetEvent on chunk_read failed. Error %lu\n", osGetLastSystemError());
        }

#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "get-----> Set chunk_read\n");
        }

#endif

        m_pSMMutex->Unlock();
    } // end while buffer isn't full

    return dwBytesRead;
}


gtUInt32 SharedMemoryManager::Peek(void* pOut, unsigned long dwBufferSize)
{
    if (pOut == NULL ||
        dwBufferSize == 0)
    {
        return 0;
    }

    void* pCopy = pOut;     //<-- this pointer will increment along the buffer as chunks are copied
    gtUInt32 dwBytesRead = 0;

    bool bFirstPass = true;

    // the expected total size is written at the top of each chunk, followed by the size of the chunk.
    // we'll use the first "total size" value as the expected total size of the entire buffer
    gtUInt32 dwExpectedTotalSize = 0;

    if (dwBytesRead < dwExpectedTotalSize || bFirstPass)
    {
        // wait for a chunk to be written
        if (false == m_pChunkWritten->Wait())
        {
            Log(logERROR, "Error occurred while waiting for chunk written:%d\n", osGetLastSystemError());
            return dwBytesRead;
        }

        if (false == m_pSMMutex->Lock())
        {
            Log(logERROR, "Error occurred while waiting for sm mutex. Error %lu\n", osGetLastSystemError());
            m_pChunkWritten->Reset();
            return dwBytesRead;
        }

        char* ptr = (char*) FindGetLocation();

        if (ptr == NULL)
        {
            // since chunkWritten was signaled, there should definitely be data to get
            Log(logERROR, "Unable to find get location. Error %lu\n", osGetLastSystemError());
            m_pChunkWritten->Reset();
            m_pSMMutex->Unlock();
            return dwBytesRead;
        }

        if (bFirstPass)
        {
            dwExpectedTotalSize = ((gtUInt32*) ptr)[ 0 ];

            // if the expected total size is greater than the supplied buffer, log an error and return.
            // since this means there was a problem with our system, I'm purposefully not filling the buffer with "as much data as we could"
            if (dwExpectedTotalSize > dwBufferSize)
            {
                Log(logERROR, "First pass: buffer (%lu bytes) not large enough to hold next message (%lu bytes).\n", dwBufferSize, dwExpectedTotalSize);
                m_pChunkWritten->Reset();
                m_pSMMutex->Unlock();
                return 0;
            }

            bFirstPass = false;
        }

        // the first value in the total size of the message
        // this should be the same as when we read it the first time (dwExpectedTotalSize)
        gtUInt32 dwTotalBufferSize = ((gtUInt32*) ptr)[ 0 ];

        // store the expected total size as a check to make sure we are still reading for the same response
        if (dwTotalBufferSize != dwExpectedTotalSize)
        {
            // log the error and break, so that the already read data is returned and we don't interfere with the next response
            Log(logERROR, "Response reading for buffer of size %lu started reading for another buffer of size %lu\n", dwExpectedTotalSize, dwTotalBufferSize);
            m_pChunkWritten->Reset();
            m_pSMMutex->Unlock();
            return dwBytesRead;
        }

        // the buffer size is valid compared to the expected size
        // so now we can move past that data.
        MOVEPTR(ptr, DWORD_SIZE);

        // read the size of the next chunk and add it to the number of bytes read
        // this better be less then the total size of the buffer
        gtUInt32 dwChunkSize = ((gtUInt32*) ptr)[ 0 ];

        // NOTE: if this happens, it is a very bad situation, means something else is wrong in the system
        // the steps here are to try to correct it.
        if (dwBytesRead + dwChunkSize > dwExpectedTotalSize)
        {
            Log(logASSERT, "Num bytes read (%lu) > Expected size (%lu)\n", dwBytesRead + dwChunkSize, dwExpectedTotalSize);
            PsAssert(!(dwBytesRead + dwChunkSize > dwExpectedTotalSize));
            dwChunkSize = std::min(dwExpectedTotalSize - dwBytesRead, dwChunkSize);
        }

        // move past the chunk size
        MOVEPTR(ptr, DWORD_SIZE);

        // copy the data from the shared memory into the input buffer,
        // copy into the current location based on chunk sizes
        memcpy_s(pCopy, dwBufferSize, ptr, dwChunkSize);

        dwBytesRead += dwChunkSize;

#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "get-----> SM current size is %lu\t(-%lu)\n", m_pHeader->dwCurrSize, (dwChunkSize + BUFFER_HEADER_SIZE));
            Log(logMESSAGE, "get-----> SM read offset  is %lu\n", m_pHeader->dwReadOffset);
            Log(logMESSAGE, "get-----> SM write offset is %lu\n", m_pHeader->dwWriteOffset);
        }

#endif

        m_pSMMutex->Unlock();
    } // end while buffer isn't full

    return dwBytesRead;
}


//--------------------------------------------------------------------------
/// Returns the size of the next buffer in bytes.
///
/// \return number of bytes in the next buffer; 0 if there is no buffer or
///   if an error occurred
//--------------------------------------------------------------------------
gtUInt32 SharedMemoryManager::GetNextBufferSize()
{
    if (false == m_pSMMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting for sm mutex. Error %lu\n", osGetLastSystemError());
        return 0;
    }

    PsAssert(m_pHeader);

    if (m_pHeader->dwCurrSize == 0)
    {
        m_pSMMutex->Unlock();
        return 0;
    }

    char* ptr = (char*) FindGetLocation();

    if (ptr == NULL)
    {
        m_pSMMutex->Unlock();
        return 0;
    }

    gtUInt32 dwNextBufferSize = ((gtUInt32*) ptr)[ 0 ];

    m_pSMMutex->Unlock();

    return dwNextBufferSize;
}

//--------------------------------------------------------------------------
/// Returns the amount of data (in bytes) that is currently in the shared
/// memory. Unless the shared memory is full, this is not the total size of
/// the shared memory.
///
/// \return number of bytes currently used in the shared memory
//--------------------------------------------------------------------------
gtUInt32 SharedMemoryManager::GetSize()
{
    PsAssert(m_pHeader);
    return m_pHeader->dwCurrSize;
}

//=============================================================================
///         Private Member Functions
//=============================================================================


//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \param dwNumBytes number of bytes that need to be put into the memory
/// \param dwNumBuffers the number of separate buffers that need to be put
///
/// \return true if the SM is locked for writing the data; false if there was an error
//--------------------------------------------------------------------------
bool SharedMemoryManager::LockPut(unsigned long dwNumBytes, unsigned long dwNumBuffers)
{
    if (dwNumBytes == 0)
    {
        Log(logWARNING, "Trying to write 0 size buffer into Shared Memory\n");
        return false;
    }

    if (false == m_pWriteMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return false;
    }

    void* pPtr = NULL;
    unsigned long dwTmp = 0;

    if (!FindPutLocation(dwNumBytes + (BUFFER_HEADER_SIZE * dwNumBuffers), pPtr, dwTmp))
    {
        UnlockPut();
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------
/// Releases the mutex after a call to LockPut
//--------------------------------------------------------------------------
void SharedMemoryManager::UnlockPut()
{
    m_pWriteMutex->Unlock();
}

//--------------------------------------------------------------------------
/// Waits on and locks the mutex
///
/// \return false if there was an error waiting on the mutex; true otherwise
//--------------------------------------------------------------------------
bool SharedMemoryManager::LockGet()
{
    if (false == m_pReadMutex->Lock())
    {
        Log(logERROR, "Error occurred while waiting :%d\n", osGetLastSystemError());
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------
/// Releases the mutex after a call to LockGet
//--------------------------------------------------------------------------
void SharedMemoryManager::UnlockGet()
{
    m_pReadMutex->Unlock();
}

//--------------------------------------------------------------------------
/// Tries to find a valid location to store dwNumBytes in the shared memory.
/// \pre LockPut has been called successfully
/// \param dwNumBytes the number of bytes that need to be put into the
///    shared memory
/// \param rpPutLocation
/// \param rdwChunkSize
/// \return true if a large enough location is found; false otherwise
//--------------------------------------------------------------------------
bool SharedMemoryManager::FindPutLocation(unsigned long dwNumBytes, void*& rpPutLocation, unsigned long& rdwChunkSize)
{
    PsAssert(m_pHeader != NULL);
    PsAssert(m_pPool != NULL);

    if (m_pHeader == NULL || m_pPool == NULL)
    {
        return false;
    }

    if (m_pHeader->dwCurrSize == 0)
    {
        Reset();
    }

    unsigned long dwMaxSize = m_pHeader->dwEnd - m_pHeader->dwStart;

    if (dwMaxSize - m_pHeader->dwCurrSize <= BUFFER_HEADER_SIZE)
    {
        // there isn't even enough room for the header information
        Log(logWARNING, "Shared memory %s doesn't have enough room for header information. Hopefully some reads will happen and free up some more space, then try again.\n", m_strName);
        Log(logWARNING, "Max size is %lu, current size is %lu, buffer header size is %lu\t(put)\n", dwMaxSize, m_pHeader->dwCurrSize, BUFFER_HEADER_SIZE);
        return false;
    }

    // make sure the write offset isn't at or near the end
    if (m_pHeader->dwWriteOffset + BUFFER_HEADER_SIZE >= m_pHeader->dwEnd)
    {
        // it is too close to even write the header in, wrap it around

        // set this value to NULL to indicate to the reader that the offsets have wrapped
        *(m_pPool + m_pHeader->dwWriteOffset) = '\0';

        // add the skipped space to the "currSize" of the buffer so that we don't
        // consider this as available space
        gtUInt32 dwSkippedSpace = dwMaxSize - m_pHeader->dwWriteOffset;
        m_pHeader->dwCurrSize += dwSkippedSpace;
#ifdef DEBUG_SHARED_MEM

        if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
        {
            Log(logMESSAGE, "--------> SM current size is %lu\t(+%lu)\t(add wrapped)\n", m_pHeader->dwCurrSize, dwSkippedSpace);
        }

#endif

        // move writeOffset back to the beginning
        m_pHeader->dwWriteOffset = 0;
    }

    // if the reading offset is higher than the writing offset,
    // that means that the writing has looped around, but reading has not
    // make sure we don't write over the unread data
    if (m_pHeader->dwReadOffset > m_pHeader->dwWriteOffset)
    {
        // in this case, the chunk size can be the minimum of either 1) space between the read and write offsets, minus the amount of space needed for a header or 2) the input data size
        rpPutLocation = m_pPool + m_pHeader->dwWriteOffset;
        rdwChunkSize = std::min<unsigned long>((m_pHeader->dwReadOffset - m_pHeader->dwWriteOffset) - BUFFER_HEADER_SIZE, dwNumBytes);
        return true;
    }
    else // read offset is behind the writing offset
    {
        // in this case, the chunk size can be the minimum of either 1) the space between the write offset and the end of the buffer or 2) the input data size
        rpPutLocation = m_pPool + m_pHeader->dwWriteOffset;
        rdwChunkSize = std::min<unsigned long>((m_pHeader->dwEnd - m_pHeader->dwWriteOffset - m_pHeader->dwStart) - BUFFER_HEADER_SIZE, dwNumBytes);
        return true;
    }
}

//--------------------------------------------------------------------------
/// Returns the memory address from which the next Get should be performed.
/// \pre LockGet has been called successfully
/// \return NULL if there is no data to read; a valid address otherwise.
//--------------------------------------------------------------------------
void* SharedMemoryManager::FindGetLocation()
{
    PsAssert(m_pHeader != NULL);
    PsAssert(m_pPool != NULL);

    // make sure there is data to read
    if (m_pHeader->dwCurrSize == 0)
    {
        return NULL;
    }

    // make sure we are not pointing to empty data
    if (*(m_pPool + m_pHeader->dwReadOffset) == '\0')
    {
        // check to see if the writeOffset is behind than the readOffset which
        // means that the writeOffset wrapped around the shared memory and the
        // ReadOffset will need to do the same and correct the "CurrSize"
        if (m_pHeader->dwWriteOffset < m_pHeader->dwReadOffset)
        {
            // adjust the "CurrSize"
            gtUInt32 dwSkippedSpace = m_pHeader->dwEnd - m_pHeader->dwReadOffset - m_pHeader->dwStart;
            m_pHeader->dwCurrSize -= dwSkippedSpace;

#ifdef DEBUG_SHARED_MEM

            if (strcmp(m_strName, DEBUG_SHARED_MEM) == 0)
            {
                Log(logMESSAGE, "--------> SM current size is %lu\t(-%lu)\t(sub wrapped)\n", m_pHeader->dwCurrSize, dwSkippedSpace);
            }

#endif

            // move the read offset
            m_pHeader->dwReadOffset = 0;
        }
    }

    // we should now be in a valid place to read
    return m_pPool + m_pHeader->dwReadOffset;
}
