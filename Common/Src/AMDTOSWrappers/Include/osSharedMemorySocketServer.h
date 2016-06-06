//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocketServer.h ------------------------------

//Yaki 6 / 5 / 2005:
//osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
//replaced by osPipeSocket.

/*
#ifndef __OSSHAREDMEMORYSOCKETSERVER
#define __OSSHAREDMEMORYSOCKETSERVER

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osSharedMemorySocket.h>

// The default size of a shared memory communication buffer (10K):
#define OS_SM_COMMUNICATION_DEFAULT_BUFF_SIZE 10240


// ----------------------------------------------------------------------------------
// Class Name:           osSharedMemorySocketServer : public osSharedMemorySocket
// General Description:
//   The server side of a shared memory socket.
//   Is responsible for creating the shared memory buffer and waiting for a
//   client connection.
//
// Author:      AMD Developer Tools Team
// Creation Date:        17/8/2005
// ----------------------------------------------------------------------------------
class OS_API osSharedMemorySocketServer : public osSharedMemorySocket
{
public:
    osSharedMemorySocketServer(const gtString& sharedMemoryObjectName, int communicationBufferSize = OS_SM_COMMUNICATION_DEFAULT_BUFF_SIZE);
    virtual ~osSharedMemorySocketServer();

    bool waitForClientConnection();

    // Overrides osSocket:
    virtual bool open();
    virtual bool close();

    // Overrides osSharedMemorySocket:
    virtual void updateSharedMemoryReleatedMembers();

private:
    bool generateUniqueTempFileName(osFilePath& filePath);
    bool createMemoryMappedFile();
    bool deleteMemoryMappedFile();
    bool createSharedMemoryObject();
    void initializeSharedMemoryHeaderFields();

private:
    // The path of the memory mapped file:
    osFilePath _memoryMappedFilePath;

    // The memory mapped file handle:
    osFileHandle _memoryMappedFileHandle;
};


#endif  // __OSSHAREDMEMORYSOCKETSERVER
*/