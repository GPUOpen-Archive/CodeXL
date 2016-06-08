//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocket.h ------------------------------


//osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
//replaced by osPipeSocket.

/*

#ifndef __OSSHAREDMEMORYSOCKET
#define __OSSHAREDMEMORYSOCKET

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osSharedMemorySocket : public osSocket
// General Description:
//   A socket that is implemented by the use of shared memory.
//   A shared memory socket enables effecient communication between two processes
//   that reside on the same machine.
//
// Author:      AMD Developer Tools Team
// Creation Date:        17/8/2005
// ----------------------------------------------------------------------------------
class OS_API osSharedMemorySocket : public osSocket
{
public:
    osSharedMemorySocket(const gtString& sharedMemoryObjectName);
    virtual ~osSharedMemorySocket();

    // Overrides osChannel:
    virtual osChannelType channelType() const;

    // Overrides osSocket:
    virtual bool isOpen() const;
    virtual bool write(const gtByte* pDataBuffer, unsigned long dataSize);
    virtual bool read(gtByte* pDataBuffer, unsigned long dataSize);

protected:
    // Enumerates the fields that reside within the shared memory header:
    // (All fields are of type int)
    enum osSharedMemoryHeaderFields
    {
        OS_SM_IS_COMMUNICATION_OPEN,        // Contains 1 iff the socket communication is open.

        OS_SM_COMMUNICATION_BUFFER_SIZE,    // A single communication buffer size (we hold two
                                            // communication buffers: client to server and server to client),

        OS_SM_SERVER_TO_CLIENT_FREE_SPACE,  // Available space in the server to client buffer.
        OS_SM_CLIENT_TO_SERVER_FREE_SPACE,  // Available space in the client to server buffer.

        OS_SM_SERVER_WRITE_POS,             // The server current write location within the server to client buffer.
        OS_SM_CLIENT_READ_POS,              // The client current read location within the server to client buffer.

        OS_SM_CLIENT_WRITE_POS,             // The client current write location within the client to server buffer.
        OS_SM_SERVER_READ_POS,              // The client current read location within the client to server buffer.

        OS_SM_SERVER_TO_CLIENT_BUFF_LOCKED, // Contains 1 iff the server to client buffer is currently locked
                                            // by a working  thread (for synchronization reasons).
        OS_SM_CLIENT_TO_SERVER_BUFF_LOCKED, // Contains 1 iff the client to server buffer is currently locked.
                                            // by a working  thread (for synchronization reasons).

        OS_SM_HEADER_FIELDS_AMOUNT          // The amount of header fields.
    };

protected:
    void clearSharedMemoryRelatedMembers();
    bool mapFileMappingObjectIntoVirtualMemory();
    int sharedMemHeaderSize() const;

    void lockBufferResources(int* pBufferLocker);
    void unlockBufferResources(int* pBufferLocker);

    bool waitForAvailableReadingData(int dataSize, long timeout);
    bool waitForAvailableSpaceToWriteData(int dataSize, long timeout);
    void waitForAnotherThread(int& waitedIntervalsCount);

    // Must be implemented by sub-classes:
    virtual void updateSharedMemoryReleatedMembers() = 0;

protected:
    // The shared memory object name:
    // (File mapping object in Win32 terminology):
    gtString _sharedMemoryObjectName;

    // A handle to the shared memory object:
     osFileHandle _sharedMemoryObjectHandle;

    // The outgoing data free space:
    int* _pIsCommunicationOpen;

    // A pointer to the shared memory block in this process address space:
    gtByte* _pSharedMemory;

    // The size of a single communication buffer:
    int _communicationBufferSize;

    // A pointer to my outgoing data buffer:
    gtByte* _pMyOutgoingDataBuff;

    // A pointer to my incoming data buffer:
    gtByte* _pMyIncomingDataBuff;

    // The outgoing data free space:
    int* _pMyOutgoingDataBuffFreeSpace;

    // The incoming data free space:
    int* _pMyIncomingDataBuffFreeSpace;

    // Contains true iff my outgoing buffer is currently locked
    // by a working thread (for synchronization reasons):
    int* _pMyOutgoingBuffLocked;

    // Contains true iff my incoming buffer is currently locked
    // by a working thread (for synchronization reasons):
    int* _pMyIncomingBuffLocked;

    // My current write location:
    int* _pMyWritePos;

    // My current read location:
    int* _pMyReadPos;

    // The other socket side current write location:
    int* _pOtherWritePos;

    // The other socket side current read location:
    int* _pOtherReadPos;
};
*/

#endif  // __OSSHAREDMEMORYSOCKET
