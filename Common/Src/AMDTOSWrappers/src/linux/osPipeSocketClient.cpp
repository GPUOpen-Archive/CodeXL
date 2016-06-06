//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketClient.cpp
///
//=====================================================================

//------------------------------ osPipeSocketClient.cpp ------------------------------

// POSIX:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>

// A pipe name that represents a pipe that was created using 2 open OS pipes:
static gtString stat_osGiveOSPipeSocketName(L"Pipe socket built from given OS pipes");


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::osPipeSocketClient
// Description: Constructor
// Arguments:   pipeName - The pipe name.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocketClient::osPipeSocketClient(const gtString& pipeName, const gtString& socketName)
    : osPipeSocket(pipeName, socketName)
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::osPipeSocketClient
// Description: Constructor - Initializes this pipes wrapper class from 2 open OS pipes.
// Arguments:   incomingPipe - An open incoming pipe file descriptor.
//                     outgoingPipe - An open outgoing pipe file descriptor.
// Author:      AMD Developer Tools Team
// Date:        6/2/2008
// ---------------------------------------------------------------------------
osPipeSocketClient::osPipeSocketClient(osPipeHandle incomingPipe, osPipeHandle outgoingPipe, const gtString& socketName)
    : osPipeSocket(stat_osGiveOSPipeSocketName, socketName)
{
    _incomingPipe = incomingPipe;
    _outgoingPipe = outgoingPipe;

    // Print a diagnotic message with this data:
    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg;
        logMsg.appendFormattedString(L"Created pipe client at address %p.\nIn: FD: %d\nOut: FD: %d",
                                     this, (int)_incomingPipe, (int)_outgoingPipe);

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }

    _isOpen = true;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::~osPipeSocketClient
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocketClient::~osPipeSocketClient()
{
    // Close the pipe connection and release all used resources:
    this->close();
}

// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::open
// Description: Connects to the pipe to the pipe server.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocketClient::open()
{
    // If this class was created from 2 open OS pipes:
    if (_pipeName == stat_osGiveOSPipeSocketName)
    {
        // The OS pipes are already open.
    }
    else
    {
        osPipeHandle originalIn = _incomingPipe;
        osPipeHandle originalOut = _outgoingPipe;

        // Close the pipe if it was open before:
        close();

        // Calculate the pipe FIFO files paths:
        gtString incomingPipeFilePath;
        gtString outgoingPipeFilePath;
        osPipeSocket::getPipeFIFOFilesPaths(outgoingPipeFilePath, incomingPipeFilePath);

        // Open the outgoing pipe:
        _outgoingPipe = ::open(outgoingPipeFilePath.asUTF8CharArray(), O_WRONLY);

        // Open the incoming pipe for reading:
        _incomingPipe = ::open(incomingPipeFilePath.asUTF8CharArray(), O_RDONLY);

        // Print a diagnotic message with this data:
        if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg;
            logMsg.appendFormattedString(L"Opened pipe client at address %p.\nIn: Old FD: %d, FD: %d, Path: %ls\nOut: Old FD: %d, FD: %d, Path: %ls",
                                         this, (int)originalIn, (int)_incomingPipe, incomingPipeFilePath.asCharArray(), (int)originalOut, (int)_outgoingPipe, outgoingPipeFilePath.asCharArray());

            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
        }
    }

    // Verify that the OS pipes were opened successfully:
    GT_IF_WITH_ASSERT((_incomingPipe != -1) && (_outgoingPipe != -1))
    {
        _errorsCount = 0;
        _isOpen = true;
    }

    return _isOpen;
}

