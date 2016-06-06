//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocket.cpp ------------------------------

Yaki 6 / 5 / 2005:
osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
replaced by osPipeSocket.

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <common/osStringConstants.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osSharedMemorySocket.h>


/*

  Shared memory sockets implementation notes:
   ==========================================
   The shared memory is build out of a header section that contains managment data
   and two data buffers:
   - Server to client - A buffer that holds data sent from the socket server to the
                        socket client.
   - Client to server - A buffer that holds data sent from the socket client to the
                        socket server.

   The buffers are ordered in a cyclic manner (when the write pointer reaches the
   buffer end, it continues writing on the buffer beginning.

   The header section contains aid data like: the size of a single data buffer,
   current read and write pointers, etc.

   We use Win32 InterlockedExchange functions for client server synchronization. These
   functions should be much faster than semaphores in Win32 implementation.

   The shared memory layout is as follows:
    -------------------------------------------------------------------------
   | Header section  |   server to client buffer  |  client to server buffer |
    -------------------------------------------------------------------------


   Bibliography:
   ============
   - "Loop buffer" Code Project article: implements most of the above ideas under Linux.
      (http://www.codeproject.com/threads/loop_buffer_-_Linux_versi.asp)

   - Win32 shared memory examples can be found in the MSDN articles:
     * "Creating Named Shared Memory" (one of the examples in CreateFileMapping documentation).
     * "File Mapping" (Windows Development -> Windows base services -> Files and I/O ->
                       SDK Documentation -> Storage -> Storage Overview -> File Managment ->
                       File Mapping)

  -  "A Fast Mostly User Mode Inter-Process Mutex" Code Project article: Uses Win32
     InterlockedExchange functions for process syncronizations.
     (http://www.codeproject.com/threads/opbmutex.asp)


*/



// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::osSharedMemorySocket
// Description: Constructor
// Arguments:   sharedMemoryObjectName - The shared memory object name.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocket::osSharedMemorySocket(const gtString& sharedMemoryObjectName)
    : _sharedMemoryObjectName(sharedMemoryObjectName)
{
    // Initialize shared memory related members:
    clearSharedMemoryRelatedMembers();
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::~osSharedMemorySocket
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
osSharedMemorySocket::~osSharedMemorySocket()
{
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::channelType
// Description: Returns my channel type.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
osChannel::osChannelType osSharedMemorySocket::channelType() const
{
    // This is a binary channel:
    return OS_BINARY_CHANNEL;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::isOpen
// Description: Returns true iff the shared memory socket is opened.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::isOpen() const
{
    bool retVal = false;

    // If the "is communication open" pointer was connected:
    if (_pIsCommunicationOpen != NULL)
    {
        // If the communication is open:
        retVal = (*_pIsCommunicationOpen == 1);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::write
// Description: Writes a block of data into the shared memory socket.
// Arguments:   pDataBuffer - A pointer to a buffer that contains the
//                            data to be written.
//              dataSize - The data size.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// Implementation notes:
//   The data buffer is cyclic, therefore, we write the data in 2 chunks:
//   - All data that fits until we reach the end of the buffer.
//   - All the remaining data (if any) will be written from the buffer begin point.
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::write(const gtByte* pDataBuffer, unsigned long dataSize)
{
    bool retVal = false;

    // Verify that the socket is open:
    if (isOpen())
    {
        // Wait for enough space to be available in my outgoing data:
        bool hasEnoughSpaceToWriteData = waitForAvailableSpaceToWriteData(dataSize, _writeOperationTimeOut);

        // If after waiting the time interval, we still don't have enough space available:
        if (!hasEnoughSpaceToWriteData)
        {
            // Do not write the data and trigger an assertion:
            GT_ASSERT(0);
        }
        else
        {
            // Lock / wait for my outgoing buffer resources:
            lockBufferResources(_pMyOutgoingBuffLocked);

            // Update the outgoing buffer free space:
            *_pMyOutgoingDataBuffFreeSpace -= dataSize;

            // Calculate the first and second chunk sizes (see "implementation notes" above):
            int firstChunkSize = min(int(dataSize), (_communicationBufferSize - *_pMyWritePos));
            int secondChunkSize = dataSize - firstChunkSize;

            // Copy the first data chunk:
            void* pWriteLocation = _pMyOutgoingDataBuff + *_pMyWritePos;
            memcpy(pWriteLocation, pDataBuffer, firstChunkSize);

            // If we need a second chunk:
            if (secondChunkSize > 0)
            {
                // Copy the second data chunk (from the beginning of our outgoing buffer):
                memcpy(_pMyOutgoingDataBuff, (pDataBuffer + firstChunkSize), secondChunkSize);
                *_pMyWritePos = secondChunkSize;
            }
            else
            {
                *_pMyWritePos += dataSize;
            }

            // If my next write position passed the end of the buffer:
            if (_communicationBufferSize <= *_pMyWritePos)
            {
                *_pMyWritePos = 0;
            }

            // Release my outgoing buffer resources:
            unlockBufferResources(_pMyOutgoingBuffLocked);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::read
// Description:
//   Reads a block of data from the shared memory socket.
//
// Arguments:   pDataBuffer - A pointer to a buffer that will receive the
//                            read data.
//              dataSize - The amount of data to be read.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2005
// Implementation notes:
//   The data buffer is cyclic, therefore, we read the data in 2 chunks:
//   - All data that fits until we reach the end of the buffer.
//   - All the remaining data (if any) will be read from the buffer begin point.
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::read(gtByte* pDataBuffer, unsigned long dataSize)
{
    bool retVal = false;

    // Verify that the socket is open
    if (isOpen())
    {
        // Wait for data to be available for reading:
        bool isEnoughDataAvailableForReading = waitForAvailableReadingData(dataSize, _readOperationTimeOut);

        // If after waiting the time interval, we still don't have data available:
        if (!isEnoughDataAvailableForReading)
        {
            // Trigger an assertion:
            GT_ASSERT_EX(false, OS_STR_ReadOperationTimeOut);
        }
        else
        {
            // Lock / wait for my incoming buffer resources:
            lockBufferResources(_pMyIncomingBuffLocked);

            // Calculate the first and second chunk sizes (see "implementation notes" above):
            int firstChunkSize = min(int(dataSize), (_communicationBufferSize - *_pMyReadPos));
            int secondChunkSize = dataSize - firstChunkSize;

            // Read the first data chunk:
            void* pReadLocation = _pMyIncomingDataBuff + *_pMyReadPos;
            memcpy(pDataBuffer, pReadLocation, firstChunkSize);

            // If we need a second chunk:
            if (secondChunkSize > 0)
            {
                // Copy the second data chunk (from the beginning of our incoming buffer):
                memcpy((pDataBuffer + firstChunkSize), _pMyIncomingDataBuff, secondChunkSize);
                *_pMyReadPos = secondChunkSize;
            }
            else
            {
                *_pMyReadPos += dataSize;
            }

            // If my next read position passed the end of the buffer:
            if (_communicationBufferSize <= *_pMyReadPos)
            {
                *_pMyReadPos = 0;
            }

            // Update the incoming buffer free space:
            *_pMyIncomingDataBuffFreeSpace += dataSize;

            // Release my incoming buffer resources:
            unlockBufferResources(_pMyIncomingBuffLocked);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::clearSharedMemoryRelatedMembers
// Description: Clears shared memory related members.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocket::clearSharedMemoryRelatedMembers()
{
    _communicationBufferSize = 0;
    _pSharedMemory = NULL;
    _pIsCommunicationOpen = NULL;
    _sharedMemoryObjectHandle = NULL;
    _pMyOutgoingDataBuff = NULL;
    _pMyIncomingDataBuff = NULL;
    _pMyOutgoingBuffLocked = NULL;
    _pMyIncomingBuffLocked = NULL;
    _pMyOutgoingDataBuffFreeSpace = NULL;
    _pMyIncomingDataBuffFreeSpace = NULL;
    _pMyWritePos = NULL;
    _pMyReadPos = NULL;
    _pOtherWritePos = NULL;
    _pOtherReadPos = NULL;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocketServer::mapFileMappingObjectIntoVirtualMemory
// Description: Maps the file mapping object into this process virtual memory.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::mapFileMappingObjectIntoVirtualMemory()
{
    bool retVal = false;

    // Map the file mapping object into this process virtual memory address space:
    _pSharedMemory = (gtByte*)(::MapViewOfFile(_sharedMemoryObjectHandle, FILE_MAP_ALL_ACCESS,
                                               0, 0, 0));

    if (_pSharedMemory != NULL)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::sharedMemHeaderSize
// Description: Calculates and returns the shared memory header size.
// Author:      AMD Developer Tools Team
// Date:        22/8/2005
// ---------------------------------------------------------------------------
int osSharedMemorySocket::sharedMemHeaderSize() const
{
    int retVal = 0;

    // All fields are currently int fields:
    retVal = sizeof(int) * OS_SM_HEADER_FIELDS_AMOUNT;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::lockBufferResources
// Description:
//   Lock buffer resources from other threads usage. If the buffer resources are
//   already locked, wait until they are free.
// Arguments:   pBufferLocker - The buffer locker variable pointer.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocket::lockBufferResources(int* pBufferLocker)
{
    // We convert int* to long* which have the same size under win32.
    // When porting to other operating systems, we have to make sure that this
    // operation is valid. Therefore, under "non approved" operating systems, we
    // will generate a compilation error:
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
#error Unsupported build target!
#endif

    // Set the locker variable value to 1 = locked:
    int waitedIntervalsCount = 0;

    while (::InterlockedExchange((long*)pBufferLocker, 1) != 0)
    {
        // The locker variable value was 1 before we set it (I.E: it
        // is currently locked by another thread), wait a bit and try again:
        waitForAnotherThread(waitedIntervalsCount);
    }
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::unlockBufferResources
// Description: Release buffer resources for other threads usage.
// Arguments:   pBufferLocker - The buffer locker variable pointer.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocket::unlockBufferResources(int* pBufferLocker)
{
    // We convert int* to long* which have the same size under win32.
    // When porting to other operating systems, we have to make sure that this
    // operation is valid. Therefore, under "non approved" operating systems, we
    // will generate a compilation error:
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
#error Unsupported build target!
#endif

    // Atomically set the locker variable to 0 = unlocked:
    ::InterlockedExchange((long*)pBufferLocker, 0);
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::waitForAvailableReadingData
// Description: Waits for data to be available for reading.
// Arguments:   timeout - Time interval to wait until data is available
//                        for reading.
// Return Val:  bool - true iff the data size in the incoming buffer is
//                     bigger than the input dataSize.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::waitForAvailableReadingData(int dataSize, long timeout)
{
    // Check if we have enough data to read:
    int dataAvailableForReading = (_communicationBufferSize - *_pMyIncomingDataBuffFreeSpace);
    bool hasEnoughDataToRead = (dataSize <= dataAvailableForReading);

    // If there is not enough data available in the socket:
    if (!hasEnoughDataToRead)
    {
        // Start measuring our wait time:
        osStopWatch stopWatch;
        stopWatch.start();

        // We will wait for data to arrive (until the timeout interval elapses):
        long waitedSoFarMSec = 0;
        int waitedIntervalsCount = 0;

        while (!hasEnoughDataToRead && (waitedSoFarMSec < timeout))
        {
            // Suspend this thread waiting for other threads to free space:
            waitForAnotherThread(waitedIntervalsCount);

            double waitedSoFarSec = 0;
            stopWatch.getTimeInterval(waitedSoFarSec);
            waitedSoFarMSec = long(waitedSoFarSec * 1000);

            dataAvailableForReading = (_communicationBufferSize - *_pMyIncomingDataBuffFreeSpace);
            hasEnoughDataToRead = (dataSize <= dataAvailableForReading);
        }
    }

    return hasEnoughDataToRead;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::waitForAvailableSpaceToWriteData
// Description: Waits until we have enough space in the socket to write
//              a defined size of data.
// Arguments:   dataSize - The size of the data to be written.
//              timeout - Time interval to wait until space is available
//                        for writing the data.
// Return Val:  bool - true iff there is enough space in the outgoing buffer
//                     to write the data in.
// Author:      AMD Developer Tools Team
// Date:        23/8/2005
// ---------------------------------------------------------------------------
bool osSharedMemorySocket::waitForAvailableSpaceToWriteData(int dataSize, long timeout)
{
    // If I don't have enough space to write the data in
    bool hasEnoughSpaceToWriteData = (dataSize < *_pMyOutgoingDataBuffFreeSpace);

    if (!hasEnoughSpaceToWriteData)
    {
        // Start measuring our wait time:
        osStopWatch stopWatch;
        stopWatch.start();

        // We will wait for this space to be available (until the timeout interval elapses):
        int waitedIntervalsCount = 0;
        long waitedSoFarMSec = 0;

        while ((!hasEnoughSpaceToWriteData) && (waitedSoFarMSec < timeout))
        {
            // Suspend this thread waiting for other threads to free space:
            waitForAnotherThread(waitedIntervalsCount);

            double waitedSoFarSec = 0;
            stopWatch.getTimeInterval(waitedSoFarSec);
            waitedSoFarMSec = long(waitedSoFarSec * 1000);

            hasEnoughSpaceToWriteData = (dataSize < *_pMyOutgoingDataBuffFreeSpace);
        }
    }

    return hasEnoughSpaceToWriteData;
}


// ---------------------------------------------------------------------------
// Name:        osSharedMemorySocket::waitForAnotherThread
// Description: Makes the current thread sleep, waiting for another thread
//              to finish its work. The more intervals we wait, the sleep
//              interval gets bigger.
// Arguments:   waitedIntervalsCount - Counts the amount of intervals we
//                                     waited so far (both input and output
//                                     parameter).
// Author:      AMD Developer Tools Team
// Date:        2/9/2005
// ---------------------------------------------------------------------------
void osSharedMemorySocket::waitForAnotherThread(int& waitedIntervalsCount)
{
    // We will wait 5 times, giving our CPU time slice to threads that have the
    // same priority as this thread, keeping this thread in a "ready to run" mode:
    if (waitedIntervalsCount < 5)
    {
        ::Sleep(0);
    }
    else
    {
        // The rest of the wait intervals will throw this thread off the CPU
        // for at least 1 msec:
        ::Sleep(1);

        // Verify that we will not overflow the waited interval count:
        waitedIntervalsCount = 6;
    }

    waitedIntervalsCount++;
}