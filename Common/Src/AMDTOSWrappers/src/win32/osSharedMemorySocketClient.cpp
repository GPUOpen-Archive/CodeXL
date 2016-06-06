//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocketClient.cpp ------------------------------

Yaki 6 / 5 / 2005:
osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
replaced by osPipeSocket.

/*

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osSharedMemorySocketClient.h>


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::osSharedMemorySocketClient
// Description: Constructor
// Arguments:   sharedMemoryObjectName - The shared memory object name.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocketClient::osSharedMemorySocketClient(const gtString& sharedMemoryObjectName)
: osSharedMemorySocket(sharedMemoryObjectName)
{
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::~osSharedMemorySocketClient
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocketClient::~osSharedMemorySocketClient()
{
    // Close the socket connection and release all used resources:
    close();
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::open
// Description: Connects to the shared memory socket and initializes the socket
//              communication.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketClient::open()
{
    // Open the socket server shared memory object:
    bool rc = openSharedMemoryObject();

    if (rc)
    {
        // Map the shared memory object into this process address space:
        rc = mapFileMappingObjectIntoVirtualMemory();

        if (rc)
        {
            // Update the shared memory related members:
            updateSharedMemoryReleatedMembers();

            // Mark that the socket connection is open:
            *_pIsCommunicationOpen = 1;
        }
    }

    // Check if we managed to establish communication with the socket server:
    bool isCommunicationOpen = isOpen();
    return isCommunicationOpen;
}



// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::close
// Description: Terminates the socket communication.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketClient::close()
{
    bool retVal = false;

    BOOL rc1 = TRUE;
    BOOL rc2 = TRUE;

    if (_pSharedMemory != NULL)
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
        // Close the handle to the socket server shared memory object:
        rc2 = ::CloseHandle(_sharedMemoryObjectHandle);
        _sharedMemoryObjectHandle = NULL;
        GT_ASSERT(rc2);
    }

    // Clear all shared memory related members:
    clearSharedMemoryRelatedMembers();

    retVal = ((rc1 != 0) && (rc2 != 0));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::updateSharedMemoryReleatedMembers
// Description: Updates shared memory related members.
// Author:      AMD Developer Tools Team
// Date:        22/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocketClient::updateSharedMemoryReleatedMembers()
{
    // Get the header section size:
    int headerSectionSize = sharedMemHeaderSize();

    // Update the "is communication open" flag:
    _pIsCommunicationOpen = (int*)(_pSharedMemory + (OS_SM_IS_COMMUNICATION_OPEN * sizeof(int)));

    // Update the buffer size:
    int* pBuffSizeField = (int*)(_pSharedMemory + (OS_SM_COMMUNICATION_BUFFER_SIZE * sizeof(int)));
    _communicationBufferSize = *pBuffSizeField;

    // Update the free space counters:
    _pMyOutgoingDataBuffFreeSpace = (int*)(_pSharedMemory + (OS_SM_CLIENT_TO_SERVER_FREE_SPACE * sizeof(int)));
    _pMyIncomingDataBuffFreeSpace = (int*)(_pSharedMemory + (OS_SM_SERVER_TO_CLIENT_FREE_SPACE * sizeof(int)));

    // Update the buffers pointers:
    _pMyIncomingDataBuff = _pSharedMemory + headerSectionSize;
    _pMyOutgoingDataBuff = _pMyIncomingDataBuff + _communicationBufferSize;

    // Update my read and write positions:
    _pMyWritePos = (int*)(_pSharedMemory + (OS_SM_CLIENT_WRITE_POS * sizeof(int)));
    _pMyReadPos = (int*)(_pSharedMemory + (OS_SM_CLIENT_READ_POS * sizeof(int)));

    // Update the "other side" read and write positions:
    _pOtherWritePos = (int*)(_pSharedMemory + (OS_SM_SERVER_WRITE_POS * sizeof(int)));
    _pOtherReadPos = (int*)(_pSharedMemory + (OS_SM_SERVER_READ_POS * sizeof(int)));

    // Update synchronization members pointers:
    _pMyOutgoingBuffLocked = (int*)(_pSharedMemory + (OS_SM_CLIENT_TO_SERVER_BUFF_LOCKED * sizeof(int)));
    _pMyIncomingBuffLocked = (int*)(_pSharedMemory + (OS_SM_SERVER_TO_CLIENT_BUFF_LOCKED * sizeof(int)));
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketClient::openSharedMemoryObject
// Description: Opens the shared memory object, created by the socket server.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocketClient::openSharedMemoryObject()
{
    bool retVal = false;

    // Open the socket server shared memory object:
    _sharedMemoryObjectHandle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _sharedMemoryObjectName.asCharArray());

    if (_sharedMemoryObjectHandle != NULL)
    {
        retVal = true;
    }

    return retVal;
}

*/