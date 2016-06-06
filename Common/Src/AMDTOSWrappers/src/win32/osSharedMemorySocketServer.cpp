//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocketServer.cpp ------------------------------

//Yaki 6 / 5 / 2005:
//osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
//replaced by osPipeSocket.

/*
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <common/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSharedMemorySocketServer.h>

// The shared memory file prefix:
#define OS_SHARED_MEM_FILE_PREFIX "GRSharedMemoryFile"

// The shared memory file extension:
#define OS_SHARED_MEM_FILE_EXTENSION "tmp"


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::osSharedMemorySocketServer
// Description: Constructor
// Arguments:   memoryMappedFilePath - The path for the shared memory socket.
//              communicationBufferSize - The size of a single shared memory buffer used
//                                        for the ipc communication.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocketServer::osSharedMemorySocketServer(const gtString& fileMappingObjectName,
                                                       int communicationBufferSize)
: osSharedMemorySocket(fileMappingObjectName), _memoryMappedFileHandle(NULL)
{
    _communicationBufferSize = communicationBufferSize;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::~osSharedMemorySocketServer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocketServer::~osSharedMemorySocketServer()
{
    // Close the socket connection and release all used resources:
    close();
}



// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::waitForClientCall
// Description: Suspends the calling thread until a client connection is
//              established.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::waitForClientConnection()
{
    // While the communication is not open:
    while (!isOpen())
    {
        // Wait for 1/2 a second:
        ::Sleep(500);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::open
// Description: Opens the server side of the shared memory socket.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::open()
{
    bool retVal = false;

    // Create the memory mapped file on disk:
    bool rc = createMemoryMappedFile();
    if (rc)
    {
        // Create the shared memory object:
        rc = createSharedMemoryObject();
        if (rc)
        {
            // Map the shared memory into this process address space:
            rc = mapFileMappingObjectIntoVirtualMemory();
            if (rc)
            {
                // Update shared memory related members:
                updateSharedMemoryReleatedMembers();

                // Initialize header fields:
                initializeSharedMemoryHeaderFields();

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::close
// Description: Closes the shared memory socket.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::close()
{
    bool retVal = false;

    BOOL rc1 = TRUE;
    BOOL rc2 = TRUE;
    bool rc3 = true;

    if (_pSharedMemory  != NULL)
    {
        // Mark that the socket connection is closed:
        *_pIsCommunicationOpen = 0;

        // Un-map the shared memory object from this process address space:
        rc1 = ::UnmapViewOfFile(_pSharedMemory);
        _pSharedMemory = NULL;
        GT_ASSERT(rc1);
    }

    if (_sharedMemoryObjectHandle != NULL)
    {
        // Delete the shared memory object:
        BOOL rc2 = ::CloseHandle(_sharedMemoryObjectHandle);
        _sharedMemoryObjectHandle = NULL;
        GT_ASSERT(rc2);
    }

    if (_memoryMappedFileHandle != NULL)
    {
        // Delete the memory mapped file:
        rc3 = deleteMemoryMappedFile();
    }

    // Clear all shared memory related members:
    clearSharedMemoryRelatedMembers();

    retVal = ((rc1 != 0) && (rc2 != 0) && rc3);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::updateSharedMemoryReleatedMembers
// Description: Updates shared memory related members.
// Author:      AMD Developer Tools Team
// Date:        22/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocketServer::updateSharedMemoryReleatedMembers()
{
    // Get the header section size:
    int headerSectionSize = sharedMemHeaderSize();

    // Update the "is communication open" flag:
    _pIsCommunicationOpen = (int*)(_pSharedMemory + (OS_SM_IS_COMMUNICATION_OPEN * sizeof(int)));

    // Update the free space counters:
    _pMyOutgoingDataBuffFreeSpace = (int*)(_pSharedMemory + (OS_SM_SERVER_TO_CLIENT_FREE_SPACE * sizeof(int)));
    _pMyIncomingDataBuffFreeSpace = (int*)(_pSharedMemory + (OS_SM_CLIENT_TO_SERVER_FREE_SPACE * sizeof(int)));

    // Update the buffers pointers:
    _pMyOutgoingDataBuff = _pSharedMemory + headerSectionSize;
    _pMyIncomingDataBuff = _pMyOutgoingDataBuff + _communicationBufferSize;

    // Update my read and write positions:
    _pMyWritePos = (int*)(_pSharedMemory + (OS_SM_SERVER_WRITE_POS * sizeof(int)));
    _pMyReadPos = (int*)(_pSharedMemory + (OS_SM_SERVER_READ_POS * sizeof(int)));

    // Update the "other side" read and write positions:
    _pOtherWritePos = (int*)(_pSharedMemory + (OS_SM_CLIENT_WRITE_POS * sizeof(int)));
    _pOtherReadPos = (int*)(_pSharedMemory + (OS_SM_CLIENT_READ_POS * sizeof(int)));

    // Update synchronization members pointers:
    _pMyOutgoingBuffLocked = (int*)(_pSharedMemory + (OS_SM_SERVER_TO_CLIENT_BUFF_LOCKED * sizeof(int)));
    _pMyIncomingBuffLocked = (int*)(_pSharedMemory + (OS_SM_CLIENT_TO_SERVER_BUFF_LOCKED * sizeof(int)));
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::generateUniqueTempFileName
// Description: Creates a unique file name under the current user temp files
//              directory.
// Arguments:   filePath - Will get the unique file name, located under the
//                         current user temp files directory.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::generateUniqueTempFileName(osFilePath& filePath)
{
    bool retVal = false;

    // Set the file directory to be the user temp directory:
    osFilePath tempDirPath(osFilePath::OS_TEMP_DIRECTORY);

    // Generate a unique file name:
    retVal = osGenerateUniqueFileName(tempDirPath, OS_SHARED_MEM_FILE_PREFIX,
                                      OS_SHARED_MEM_FILE_EXTENSION, filePath);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::createMemoryMappedFile
// Description: Creates the memory mapped file.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::createMemoryMappedFile()
{
    bool retVal = false;

    // Generate a unique file name under the temp directory:
    bool rc = generateUniqueTempFileName(_memoryMappedFilePath);
    if (rc)
    {
        // Create the shared memory file:
        _memoryMappedFileHandle = CreateFile(_memoryMappedFilePath.asString().asCharArray(),
                                             GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_NEW,
                                             FILE_ATTRIBUTE_NORMAL, NULL);

        if (_memoryMappedFileHandle != INVALID_HANDLE_VALUE)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::deleteMemoryMappedFile
// Description: Deletes the memory mapped file created by createMemoryMappedFile()
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::deleteMemoryMappedFile()
{
    bool retVal = false;

    // Close the handle to the file:
    BOOL rc1 = ::CloseHandle(_memoryMappedFileHandle);
    _memoryMappedFileHandle = NULL;
    GT_ASSERT(rc1);

    // Delete the file:
    // Notice: If the client still didn't close its shared memory object handle yet,
    //         we will fail deleting the file, because the shared memory object owns
    //         the file. Therefore, we try to few times to delete the file.
    //         Even when we fail in all deleting attempts, we don't fail this function.
    BOOL rc2 = FALSE;
    int deletionTryNo = 0;
    while (deletionTryNo < 8)
    {
        rc2 = ::DeleteFile(_memoryMappedFilePath.asString().asCharArray());
        if (rc2 != 0)
        {
            break;
        }
        else
        {
            // We didn't manage to delete the file - sleep for 1 second trying to
            // enable the spy to delete it:
            ::Sleep(1000);
        }

        deletionTryNo++;
    }

    // If we didn't manage to delete the shared memory file - output a log message:
    if (!rc2)
    {
        gtString debugLogMessage = OS_STR_FailedToDeleteSharedMemFile;
        debugLogMessage += _memoryMappedFilePath.asString();
        OS_OUTPUT_DEBUG_LOG(debugLogMessage.asCharArray(), OS_ERROR_SEVERITY);
    }

    _memoryMappedFilePath;

    // See comment above rc2:
    retVal = (rc1 != 0);
    GT_ASSERT(retVal);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::createSharedMemoryObject
// Description: Creates a shared memory object (file mapping object in Win32 terminology).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketServer::createSharedMemoryObject()
{
    bool retVal = false;

    // Calculate the required shared memory size:
    int headerSectionSize = sharedMemHeaderSize();
    int sharedMemorySize = headerSectionSize + (2 * _communicationBufferSize);

    // Create a file mapping object for the file.
    // (A file mapping object maps a file content into a shared memory object)
    _sharedMemoryObjectHandle = CreateFileMapping(_memoryMappedFileHandle,
                                                 NULL, PAGE_READWRITE,
                                                 0, sharedMemorySize,
                                                 _sharedMemoryObjectName.asCharArray());
    if (_sharedMemoryObjectHandle != NULL)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::initializeSharedMemoryHeaderFields
// Description: Initialize the shared memory header fields.
// Author:      AMD Developer Tools Team
// Date:        22/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocketServer::initializeSharedMemoryHeaderFields()
{
    // Initialize the "is communication open" flag value:
    *_pIsCommunicationOpen = 0;

    // Initialize the buffer size field:
    int* pBuffSizeField = (int*)(_pSharedMemory + (OS_SM_COMMUNICATION_BUFFER_SIZE * sizeof(int)));
    *pBuffSizeField = _communicationBufferSize;

    // Initialize the free space fields:
    *_pMyOutgoingDataBuffFreeSpace = _communicationBufferSize;
    *_pMyIncomingDataBuffFreeSpace = _communicationBufferSize;

    // Initialize read and write position fields:
    *_pMyWritePos = 0;
    *_pMyReadPos = 0;
    *_pOtherWritePos = 0;
    *_pOtherReadPos = 0;

    // Initialize synchronization members:
    *_pMyOutgoingBuffLocked = 0;
    *_pMyIncomingBuffLocked = 0;
}

*/