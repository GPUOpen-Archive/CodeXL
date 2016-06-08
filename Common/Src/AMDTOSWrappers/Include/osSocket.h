//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSocket.h
///
//=====================================================================

//------------------------------ osSocket.h ------------------------------

#ifndef __OSSOCKET
#define __OSSOCKET

// Local:
#include <AMDTOSWrappers/Include/osChannel.h>


// ----------------------------------------------------------------------------------
// Class Name:           osSocket : public osChannel
// General Description:
//   Represents an OS socket.
//   A socket enables communication between two local / remote processes.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osSocket : public osChannel
{
public:
    osSocket(const gtString& socketName) : _socketName(socketName) {};
    virtual ~osSocket();

    // Must be implemented by sub - classes:
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool isOpen() const = 0;

private:

    // Do not allow the use of the default constructor:
    osSocket();

protected:
    gtString _socketName;
};


#endif  // __OSSOCKET
