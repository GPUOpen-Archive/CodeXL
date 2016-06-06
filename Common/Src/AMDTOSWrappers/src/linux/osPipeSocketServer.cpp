//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketServer.cpp
///
//=====================================================================

//------------------------------ osPipeSocketServer.cpp ------------------------------

// POSIX:
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::osPipeSocketServer
// Description: Constructor
// Arguments:   pipeName - The pipe name.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocketServer::osPipeSocketServer(const gtString& pipeName)
    : osPipeSocket(pipeName, L"osPipeSocketServer"), _fifoFilesExists(false)
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::~osPipeSocketServer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocketServer::~osPipeSocketServer()
{
    // Close the pipe connection and release all used resources:
    this->close();
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::open
// Description: Create the server pipe, to which the client pipe will connect
//              later on.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocketServer::open()
{
    bool retVal = true;

    // Get the paths of the 2 FIFO files, used for implementing this
    // full duplex pipe:
    gtString incomingPipeFilePath;
    gtString outgoingPipeFilePath;
    osPipeSocket::getPipeFIFOFilesPaths(incomingPipeFilePath, outgoingPipeFilePath);

    // Create the pipes FIFO files:
    int rc1 = ::mkfifo(incomingPipeFilePath.asUTF8CharArray(), 0666);
    int rc2 = ::mkfifo(outgoingPipeFilePath.asUTF8CharArray(), 0666);

    GT_IF_WITH_ASSERT((rc1 == 0) && (rc2 == 0))
    {
        _fifoFilesExists = true;
        _errorsCount = 0;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::close
// Description: Terminates the pipe communication.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        1/1/2007
// ---------------------------------------------------------------------------
bool osPipeSocketServer::close()
{
    bool retVal = false;

    if (!_isOpen)
    {
        // Nothing to be done:
        retVal = true;
    }
    else
    {
        // Close the FIFO files:
        bool rc1 = osPipeSocket::close();

        if (_fifoFilesExists)
        {
            // Delete the 2 FIFO files, used for implementing this full duplex pipe:
            gtString incomingPipeFilePath;
            gtString outgoingPipeFilePath;
            osPipeSocket::getPipeFIFOFilesPaths(incomingPipeFilePath, outgoingPipeFilePath);

            int rc2 = ::unlink(incomingPipeFilePath.asUTF8CharArray());
            int rc3 = ::unlink(outgoingPipeFilePath.asUTF8CharArray());
            _fifoFilesExists = false;

            if (rc1 && (rc2 == 0) && (rc3 == 0))
            {
                retVal = true;
            }
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::waitForClientConnection
// Description: Suspends the calling thread until a client connection is
//              established.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        1/1/2007
// ---------------------------------------------------------------------------
bool osPipeSocketServer::waitForClientConnection()
{
    bool retVal = false;

    osPipeHandle originalIn = _incomingPipe;
    osPipeHandle originalOut = _outgoingPipe;

    // Close any previous connections:
    close();

    // Get the paths of the 2 FIFO files, used for implementing this
    // full duplex pipe:
    gtString incomingPipeFilePath;
    gtString outgoingPipeFilePath;
    osPipeSocket::getPipeFIFOFilesPaths(incomingPipeFilePath, outgoingPipeFilePath);

    // Open the incoming pipe for reading:
    _incomingPipe = ::open(incomingPipeFilePath.asUTF8CharArray(), O_RDONLY);

    if (_incomingPipe != -1)
    {
        // Open the outgoing pipe:
        _outgoingPipe = ::open(outgoingPipeFilePath.asUTF8CharArray(), O_WRONLY);
    }

    GT_IF_WITH_ASSERT((_incomingPipe != -1) && (_outgoingPipe != -1))
    {
        _isOpen = true;
        retVal = true;
    }

    // Print a diagnotic message with this data:
    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Opened pipe server at address %p.\nIn: Old FD: %d, FD: %d\nOut: Old FD: %d, FD: %d\nRetVal: %ls; Open: %ls",
                                     this, (int)originalIn, (int)_incomingPipe, (int)originalOut, (int)_outgoingPipe,
                                     retVal ? L"true" : L"false", _isOpen ? L"true" : L"false");

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    return retVal;
}


