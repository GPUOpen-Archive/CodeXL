//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a block of memory shared
///         between threads and processes. Implementation is OS-specific and
///         accessed through a common OS-agnostic interface
///         NOTE: There is no synchronization object (mutex, semaphore etc)
///         protecting the shared memory; It is up to the client to provide that
///         functionality.
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#elif defined (_LINUX)
    #include <string.h>
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <sys/stat.h>        // For mode constants
    #include <fcntl.h>           // For O_* constants
    #include <unistd.h>
    #include <boost/interprocess/shared_memory_object.hpp>
    #include <boost/interprocess/mapped_region.hpp>

    using namespace boost::interprocess;
#endif
#include "SharedMemory.h"
#include "defines.h"

/// Base Implementation abstract data type
class SharedMemoryImpl
{
public:

    /// Constructor
    SharedMemoryImpl()
    {
    }

    /// Destructor
    virtual ~SharedMemoryImpl()
    {
    }

    /// Open an existing or create a new shared memory
    /// \param bufferSize Size of the buffer
    /// \param mappingName Name of the memory
    virtual SharedMemory::MemStatus OpenOrCreate(int bufferSize, const char* mappingName) = 0;

    /// Open an existing shared memory
    /// \param mappingName Name of the memory to open
    virtual SharedMemory::MemStatus Open(const char* mappingName) = 0;

    /// Check to see if a named shared memory exists
    /// \param mappingName Name of teh shared memory to locate
    /// \return True if the memory exists, false if not
    virtual bool Exists(const char* mappingName) = 0;

    /// Gets a pointer to the shared memory
    /// \return
    virtual void* Get() = 0;

    /// Close the shared memopry
    virtual void Close() = 0;
};

/// Windows-specific implementation
#if defined (_WIN32)
class SharedMemoryWindows : public SharedMemoryImpl
{
public:
    /// default constructor
    SharedMemoryWindows()
        : m_hMapFile(NULL)
        , m_pMemory(NULL)
    {
    }

    /// destructor
    virtual ~SharedMemoryWindows()
    {
        Close();
    }

    //--------------------------------------------------------------------------
    /// Try to open an existing shared memory block or Create a new one if it
    /// doesn't exist
    /// \param bufferSize the size of the memory buffer needed
    /// \param mappingName the name that this memory buffer will be identified
    ///   by
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  OpenOrCreate(int bufferSize, const char* mappingName)
    {
        m_hMapFile = CreateFileMapping(
                         INVALID_HANDLE_VALUE,          // use paging file
                         NULL,                          // default security
                         PAGE_READWRITE,                // read/write access
                         0,                             // max. object size
                         bufferSize,                    // buffer size
                         mappingName);                  // name of mapping object

        DWORD dwError = GetLastError();

        // The first process to attach initializes memory
        bool mapFileCreated = (dwError == ERROR_ALREADY_EXISTS);

        if (m_hMapFile == NULL)
        {
            return SharedMemory::ERROR_CREATE;
        }

        m_pMemory = MapViewOfFile(m_hMapFile,              // handle to map object
                                  FILE_MAP_ALL_ACCESS, // read/write permission
                                  0,
                                  0,
                                  0);

        if (m_pMemory == NULL)
        {
            return SharedMemory::ERROR_MAPPING;
        }

        if (mapFileCreated)
        {
            return SharedMemory::SUCCESS_ALREADY_CREATED;
        }

        return SharedMemory::SUCCESS;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created shared memory block
    /// \param mappingName the name of the memory buffer
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  Open(const char* mappingName)
    {
        // open shared memory
        m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
                                      FALSE,              // inherit handle?
                                      mappingName);      // name of map object

        if (m_hMapFile == NULL)
        {
            return SharedMemory::ERROR_OPEN;
        }

        // Get a pointer to the file-mapped shared memory
        m_pMemory = MapViewOfFile(m_hMapFile,         // object to map view of
                                  FILE_MAP_ALL_ACCESS, // read/write access
                                  0,                   // high offset:  map from
                                  0,                   // low offset:   beginning
                                  0);                  // default: map entire file

        if (m_pMemory == NULL)
        {
            return SharedMemory::ERROR_MAPPING;
        }

        return SharedMemory::SUCCESS;
    }

    //--------------------------------------------------------------------------
    /// Does this shared memory block exist in the system already.
    /// \param mappingName the name of the memory buffer
    ///
    /// \return true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual bool  Exists(const char* mappingName)
    {
        // open shared mem
        m_hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
                                      FALSE,              // inherit handle?
                                      mappingName);      // name of map object

        if (m_hMapFile == NULL)
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Get a pointer to the shared memory
    ///
    /// \return pointer to the start of memory true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual void* Get()
    {
        return m_pMemory;
    }

    //--------------------------------------------------------------------------
    /// Close the shared memory
    //--------------------------------------------------------------------------
    virtual void Close()
    {
        if (m_pMemory)
        {
            UnmapViewOfFile(m_pMemory);
            m_pMemory = NULL;
        }

        if (m_hMapFile)
        {
            CloseHandle(m_hMapFile);
            m_hMapFile = NULL;
        }
    }

private:
    void*       m_hMapFile;             ///< MapFile handle created once at initialization.
    void*       m_pMemory;              ///< Pointer to shared copy of data in shared memory region.
};

#endif      //_WIN32

#ifdef _LINUX
/// Boost mapped_file probably won't work on Android
/// http://www.boost.org/doc/libs/1_48_0/libs/iostreams/doc/classes/mapped_file.html

/// Boost implementation using boost::interprocess::shared_memory_object
/// http://www.boost.org/doc/libs/1_47_0/doc/html/interprocess/sharedmemorybetweenprocesses.html

class SharedMemoryBoostSharedMemory : public SharedMemoryImpl
{
public:
    /// default constructor
    SharedMemoryBoostSharedMemory()
        : m_memoryObject(NULL)
        , m_mappedRegion(NULL)
    {
    }

    /// destructor
    virtual ~SharedMemoryBoostSharedMemory()
    {
        if (m_memoryObject)
        {
            delete m_memoryObject;
        }

        if (m_mappedRegion)
        {
            delete m_mappedRegion;
        }
    }

    //--------------------------------------------------------------------------
    /// Try to open an existing shared memory block or Create a new one if it
    /// doesn't exist
    /// \param bufferSize the size of the memory buffer needed
    /// \param mappingName the name that this memory buffer will be identified
    ///   by
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  OpenOrCreate(int bufferSize, const char* mappingName)
    {
        strcpy(m_memoryName, mappingName);

        if (m_memoryObject == NULL)
        {
            // First, try to open the shared memory. If that fails, create a new shared memory
            // object.
            // Exceptions are a no-no, but this constructor throws, so the exception
            // needs to be handled correctly.
            bool   justCreated = false;

            try
            {
                m_memoryObject = new shared_memory_object(open_only, mappingName, read_write);
            }
            catch (interprocess_exception&)
            {
                if (m_memoryObject == NULL)
                {
                    m_memoryObject = new shared_memory_object(open_or_create, mappingName, read_write);

                    if (m_memoryObject == NULL)
                    {
                        return SharedMemory::ERROR_CREATE;
                    }

                    justCreated = true;
                }
            }

            // if the shared memory file has just been created, set it to the correct size
            if (justCreated)
            {
                m_memoryObject->truncate(bufferSize);
            }

            // map the whole shared memory object block in this process
            m_mappedRegion = new mapped_region(*m_memoryObject, read_write);

            if (m_mappedRegion == NULL)
            {
                return SharedMemory::ERROR_MAPPING;
            }

            // if the memory block has just been created, clear the memory
            if (justCreated)
            {
                memset(m_mappedRegion->get_address(), 0, m_mappedRegion->get_size());
                return SharedMemory::SUCCESS;
            }
            else
            {
                return SharedMemory::SUCCESS_ALREADY_CREATED;
            }
        }

        return SharedMemory::SUCCESS;
    }

    //--------------------------------------------------------------------------
    /// Open a previously created shared memory block
    /// \param mappingName the name of the memory buffer
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  Open(const char* mappingName)
    {
        if (m_memoryObject == NULL)
        {
            try
            {
                m_memoryObject = new shared_memory_object(open_only, mappingName, read_write);
            }
            catch (interprocess_exception&)
            {
                return SharedMemory::ERROR_OPEN;
            }

            m_memoryObject = new shared_memory_object(open_only, mappingName, read_write);

            if (m_mappedRegion == NULL)
            {
                m_mappedRegion = new mapped_region(*m_memoryObject, read_write);
            }
        }

        return SharedMemory::SUCCESS;
    }

    //--------------------------------------------------------------------------
    /// Does this shared memory block exist in the system already.
    /// \param mappingName the name of the memory buffer
    ///
    /// \return true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual bool  Exists(const char* mappingName)
    {
        if (m_memoryObject)
        {
            return true;
        }
        else
        {
            try
            {
                shared_memory_object shm(open_only, mappingName, read_write);
                return true;
            }
            catch (interprocess_exception&)
            {
                return false;
            }
        }
    }

    //--------------------------------------------------------------------------
    /// Get a pointer to the shared memory
    ///
    /// \return pointer to the start of memory true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual void* Get()
    {
        if (m_mappedRegion)
        {
            return m_mappedRegion->get_address();
        }

        return NULL;
    }

    //--------------------------------------------------------------------------
    /// Close the shared memory
    //--------------------------------------------------------------------------
    virtual void Close()
    {
        shared_memory_object::remove(m_memoryName);
    }

private:
    shared_memory_object*  m_memoryObject;             ///< pointer to the shared memory object
    mapped_region*         m_mappedRegion;             ///< pointer to the mapped region object
    char                   m_memoryName[PS_MAX_PATH];  ///< the name used to describe this shared memory
};

/// Linux-specific (POSIX) implementation
static const int s_Mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

class SharedMemoryPosix : public SharedMemoryImpl
{
public:
    /// default constructor
    SharedMemoryPosix()
        : m_handle(-1)
        , m_memory(NULL)
        , m_owner(false)
    {
    }

    /// destructor
    /// IMPORTANT NOTE: According to the POSIX documentation regarding
    /// shm_unlink:
    ///
    /// The shm_unlink() function shall remove the name of the shared memory
    /// object named by the string pointed to by name.
    ///
    /// If one or more references to the shared memory object exist when the
    /// object is unlinked, the name shall be removed before shm_unlink()
    /// returns, but the removal of the memory object contents shall be
    /// postponed until all open and map references to the shared memory object
    /// have been removed.
    ///
    /// This basically means that anything which has a reference to the shared
    /// memory AFTER the first shm_unlink call is safe to still use their copy
    /// of the shared memory. However, any subsequent calls to Open() will
    /// fail since the shm_unlink has removed the name. Therefore, in a multi
    /// threaded, multi-process environment, it is important for all threads
    /// and processes to Open() the shared memory if they need access to it
    /// BEFORE the first shm_unlink() has been called. Calls to OpenOrCreate()
    /// will create a new shared memory object.
    virtual ~SharedMemoryPosix()
    {
        Close();

        if (m_owner)
        {
            int status = shm_unlink(m_memoryName);

            if (status == -1)
            {
#ifdef DEBUG_PRINT
                printf("Error on unlink()\n");
#endif
            }
        }
    }

    //--------------------------------------------------------------------------
    /// Try to open an existing shared memory block or Create a new one if it
    /// doesn't exist
    /// \param bufferSize the size of the memory buffer needed
    /// \param mappingName the name that this memory buffer will be identified
    ///   by
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  OpenOrCreate(int bufferSize, const char* mappingName)
    {
        bool   justCreated = false;

        // see if the memory exists already. If not, then create it
        char nameWithExtension[PS_MAX_PATH];
        sprintf_s(nameWithExtension, PS_MAX_PATH, "%s", mappingName);
        int handle = shm_open(nameWithExtension, O_RDWR, s_Mode);

        if (handle < 0)
        {
            handle = shm_open(nameWithExtension, O_RDWR | O_CREAT, s_Mode);

            if (handle < 0)
            {
#ifdef DEBUG_PRINT
                printf("Shared memory: creation error\n");
#endif
                return SharedMemory::ERROR_CREATE;
            }

            justCreated = true;
            m_owner = true;
        }

        m_handle = handle;

        if (justCreated)
        {
            // make it the right size
            ftruncate(m_handle, bufferSize);
        }

        // map the memory into this process' address space
        m_memory = mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

        if (m_memory == (void*) - 1)
        {
#ifdef DEBUG_PRINT
            printf("Shared memory: mapping error on create\n");
#endif
            Close();
            return SharedMemory::ERROR_MAPPING;
        }

        // initialize the memory if it has just been created
        m_memorySize = bufferSize;
        strcpy(m_memoryName, nameWithExtension);

        if (justCreated)
        {
            memset(m_memory, 0, m_memorySize);
            return SharedMemory::SUCCESS;
        }
        else
        {
            return SharedMemory::SUCCESS_ALREADY_CREATED;
        }
    }

    //--------------------------------------------------------------------------
    /// Open a previously created shared memory block
    /// \param mappingName the name of the memory buffer
    ///
    /// \return MapFileStatus value indicating success or the type of error
    //--------------------------------------------------------------------------
    virtual SharedMemory::MemStatus  Open(const char* mappingName)
    {
        char nameWithExtension[PS_MAX_PATH];
        sprintf_s(nameWithExtension, PS_MAX_PATH, "%s", mappingName);
        int handle = shm_open(nameWithExtension, O_RDWR, s_Mode);

        if (handle < 0)
        {
#ifdef DEBUG_PRINT
            printf("shared memory: error on open, can't open shared memory\n");
#endif
            return SharedMemory::ERROR_OPEN;
        }

        m_handle = handle;

        // get the size of the shared memory
        struct stat bufStats;
        int retVal = fstat(m_handle, &bufStats);

        if (retVal != 0)
        {
            return SharedMemory::ERROR_OPEN;
        }

        m_memorySize = bufStats.st_size;
#ifdef DEBUG_PRINT
        printf("SharedMemory::Open(), buffer size is %d\n", bufStats.st_size);
#endif
        strcpy(m_memoryName, nameWithExtension);
        m_memory = mmap(NULL, m_memorySize, PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

        if (m_memory == (void*) - 1)
        {
#ifdef DEBUG_PRINT
            printf("shared memory: error on open, can't map memory\n");
#endif
            Close();
            return SharedMemory::ERROR_MAPPING;
        }

        return SharedMemory::SUCCESS;
    }

    //--------------------------------------------------------------------------
    /// Does this shared memory block exist in the system already.
    /// \param mappingName the name of the memory buffer
    ///
    /// \return true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual bool  Exists(const char* mappingName)
    {
        char nameWithExtension[PS_MAX_PATH];
        sprintf_s(nameWithExtension, PS_MAX_PATH, "%s", mappingName);
        int handle = shm_open(nameWithExtension, O_RDWR, s_Mode);

        if (handle < 0)
        {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------------------
    /// Get a pointer to the shared memory
    ///
    /// \return pointer to the start of memory true if it exists, false otherwise
    //--------------------------------------------------------------------------
    virtual void* Get()
    {
        return m_memory;
    }

    //--------------------------------------------------------------------------
    /// Close the shared memory
    //--------------------------------------------------------------------------
    virtual void Close()
    {
        if (m_handle >= 0)
        {
            if (m_memory != NULL)
            {
                int status = munmap(m_memory, m_memorySize);

                if (status == -1)
                {
#ifdef DEBUG_PRINT
                    printf("Error on munmap()\n");
#endif
                }
            }

            close(m_handle);
            m_handle = -1;
        }
    }

private:
    char           m_memoryName[PS_MAX_PATH];  ///< the name used to describe this shared memory
    int            m_memorySize;               ///< size of shared memory buffer
    int            m_handle;                   ///< handle to shared memory buffer
    void*          m_memory;                   ///< pointer to shared memory buffer
    bool           m_owner;                    ///< did this object create the shared memory
};

#endif   // _LINUX

/// Main Implementation methods.
/// default constructor
/// Pick an implementation based on platform
SharedMemory::SharedMemory()
{
#if defined _WIN32
    m_pImpl = new SharedMemoryWindows();
#else
    m_pImpl = new SharedMemoryPosix();
#endif
}

/// destructor
SharedMemory::~SharedMemory()
{
    delete m_pImpl;
}

//--------------------------------------------------------------------------
/// Try to open an existing shared memory block or Create a new one if it
/// doesn't exist
/// \param bufferSize the size of the memory buffer needed
/// \param mappingName the name that this memory buffer will be identified
///   by
///
/// \return MapFileStatus value indicating success or the type of error
//--------------------------------------------------------------------------
SharedMemory::MemStatus  SharedMemory::OpenOrCreate(int bufferSize, const char* mappingName)
{
    return m_pImpl->OpenOrCreate(bufferSize, mappingName);
}

//--------------------------------------------------------------------------
/// Open a previously created shared memory block
/// \param mappingName the name of the memory buffer
///
/// \return MapFileStatus value indicating success or the type of error
//--------------------------------------------------------------------------
SharedMemory::MemStatus  SharedMemory::Open(const char* mappingName)
{
    return m_pImpl->Open(mappingName);
}

//--------------------------------------------------------------------------
/// Does this shared memory block exist in the system already.
/// \param mappingName the name of the memory buffer
///
/// \return true if it exists, false otherwise
//--------------------------------------------------------------------------
bool  SharedMemory::Exists(const char* mappingName)
{
    return m_pImpl->Exists(mappingName);
}

//--------------------------------------------------------------------------
/// Get a pointer to the shared memory
///
/// \return pointer to the start of memory true if it exists, false otherwise
//--------------------------------------------------------------------------
void* SharedMemory::Get()
{
    return m_pImpl->Get();
}

//--------------------------------------------------------------------------
/// Close the shared memory
//--------------------------------------------------------------------------
void  SharedMemory::Close()
{
    m_pImpl->Close();
}
