//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Shared Globals mechanism for PerfStudio.Used to send massages to the
/// graphics server plugins via shared memory.
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#elif defined (_LINUX)
    #include "WinDefs.h"
#endif
#include <AMDTOSWrappers/Include/osSystemError.h>
#include "SharedGlobal.h"
#include "Logger.h"

/// Definition
const char* SHARED_MEMORY_NAME = "PerfStudioSharedGlobals";

//-----------------------------------------------------------------------------
/// Provides access to the single instance of this class in the process. If the
/// class does not exist, it will initialize it and log any errors that occur.
/// \return pointer to the instance of this class.
//-----------------------------------------------------------------------------
SharedGlobal* SharedGlobal::Instance(void)
{
    static SharedGlobal* sg = new SharedGlobal;

    PsAssert(sg != NULL);

    if (!sg->m_bInitialized && !sg->Initialize())
    {
        LogConsole(logERROR, "Unable to create SharedGlobal data\n");
        delete sg;
        sg = NULL;
    }

    return (sg);
}


//=============================================================================
///         Public Member Functions
//=============================================================================


// Global Data Accessor Functions.
// There will be a Get & A Set function defined for each data type stored in
// the Global Shared data block.
// Note: All access to the shared data is via the SG_ macros defined in SharedGlobal.h


//-----------------------------------------------------------------------------
/// Set a path in the global data block. All paths are assumed to be PS_MAX_PATH size.
///
/// \param offset The offset from the start of the structure to the path of interest.
///    It is calculated automatically at compile time using the offsetof functionality
///    through the SG_SET_PATH macro
/// \param path The path to set at the specified offset
/// \return true if the path could be set; false otherwise
//-----------------------------------------------------------------------------
bool SharedGlobal::SetPath(size_t offset, const char* path)
{
    PsAssert(this != NULL);
    PsAssert(path != NULL);
    PsAssert(m_MapFile != NULL);
    PsAssert(m_MapFile->Get() != NULL);

    if (Lock())
    {
        strcpy_s(&((char*)m_MapFile->Get())[offset], PS_MAX_PATH, path);

        Unlock();
        return (true);
    }

    return (false);
}

//-----------------------------------------------------------------------------
/// Get a path from the global data block. All paths are assumed to be PS_MAX_PATH size.
///
/// \param offset The offset from the start of the structure to the path of interest.
/// It is calculated automatically at compile time using the offsetof functionality
/// through the SG_GET_PATH macro
///
/// \return A shadow copy of the path if maintained in the class. This allows us to directly pass back
/// a char * without concerns about locking/unlocking the shared memory region.
//-----------------------------------------------------------------------------
const char* SharedGlobal::GetPath(size_t offset)
{
    PsAssert(this != NULL);
    PsAssert(m_MapFile != NULL);
    PsAssert(m_MapFile->Get() != NULL);

    char* src = & ((char*)m_MapFile->Get())[offset];
    char* dst = & ((char*) & m_Shadow)[offset];

    size_t nSize = sizeof((&m_Shadow)[offset]);

    if (Lock())
    {
        // copy shared memory copy of string into local shadow copy
        memcpy_s(dst, nSize, src, PS_MAX_PATH);
        Unlock();
        return (dst);
    }

    return (NULL);

}

//=============================================================================
///         Private Member Functions
//=============================================================================

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
SharedGlobal::SharedGlobal()
{
    m_Mutex = NULL;
    m_MapFile = new SharedMemory();
    m_bInitialized = false;

    memset(&m_Shadow, 0, sizeof(PsSharedGlobal));
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SharedGlobal::~SharedGlobal()
{
    delete m_MapFile;

    delete m_Mutex;
}

//-----------------------------------------------------------------------------
/// Initialize all class data. This should only be called once
/// \return true if everything is properly initialized; false otherwise
//-----------------------------------------------------------------------------
bool SharedGlobal::Initialize()
{
    // Create the Mutex used to control access to the Shared Memory block
    m_Mutex = new osMutex();

    SharedMemory::MemStatus  status = m_MapFile->OpenOrCreate(sizeof(PsSharedGlobal), SHARED_MEMORY_NAME);

    if (SharedMemory::ERROR_CREATE == status)
    {
        LogConsole(logERROR, "Could not create file mapping object (%d).\n", osGetLastSystemError());
        return (false);
    }

    if (SharedMemory::ERROR_MAPPING == status)
    {
        LogConsole(logERROR, "Could not map view of file (%d).\n", osGetLastSystemError());
        return (false);
    }

    m_bInitialized = true;
    return (true);
}

//-----------------------------------------------------------------------------
/// Lock global shared memory region for exclusive access.
/// \return true if the shared memory could be locked; false otherwise
//-----------------------------------------------------------------------------
bool SharedGlobal::Lock(void)
{
    PsAssert(this != NULL);
    PsAssert(m_Mutex != NULL);

    if (m_Mutex->lock() == false)
    {
        Log(logERROR, "Error occurred while waiting for Mutex :%d\n", osGetLastSystemError());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Release global shared memory region so it can be accessed by other processes.
//-----------------------------------------------------------------------------
void SharedGlobal::Unlock(void)
{
    PsAssert(this != NULL);
    PsAssert(m_Mutex != NULL);

    m_Mutex->unlock();
}
