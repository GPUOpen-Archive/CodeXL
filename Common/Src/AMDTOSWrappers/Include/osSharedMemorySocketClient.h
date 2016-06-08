//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osSharedMemorySocketClient.h ------------------------------

//Yaki 6 / 5 / 2005:
//osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
//replaced by osPipeSocket.

/*
#ifndef __OSSHAREDMEMORYSOCKETCLIENT
#define __OSSHAREDMEMORYSOCKETCLIENT

// Local:
#include <AMDTOSWrappers/Include/osSharedMemorySocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osSharedMemorySocketClient : public osSharedMemorySocket
// General Description:
//   The client side of a shared memory socket.
//   Connects itself to a waiting shared memory socket server.
// Author:      AMD Developer Tools Team
// Creation Date:        21/8/2005
// ----------------------------------------------------------------------------------
class OS_API osSharedMemorySocketClient : public osSharedMemorySocket
{
public:
    osSharedMemorySocketClient(const gtString& sharedMemoryObjectName);
    virtual ~osSharedMemorySocketClient();

    // Overrides osSocket:
    virtual bool open();
    virtual bool close();

    // Overrides osSharedMemorySocket:
    virtual void updateSharedMemoryReleatedMembers();

private:
    bool openSharedMemoryObject();
};


#endif  // __OSSHAREDMEMORYSOCKETCLIENT
*/