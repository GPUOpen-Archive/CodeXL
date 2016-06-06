//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocket.cpp
///
//=====================================================================

//------------------------------ osPipeSocket.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeSocket.h>

// Default read / write timeouts:
// (Currently not used by our win sockets implementation)
#define OS_PIPE_SOCKET_DEFAULT_READ_TIMEOUT 180000
#define OS_PIPE_SOCKET_DEFAULT_WRITE_TIMEOUT 180000

// Maximal amount of error printouts:
#define OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS 10


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::osPipeSocket
// Description: Constructor.
// Arguments: pipeName - The pipe name.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocket::osPipeSocket(const gtString& pipeName, const gtString& socketName)
    : osSocket(socketName), _pipeName(pipeName), _isOpen(false), _isDuringPipeConnectionWait(false),
      _incomingPipe(NULL), _outgoingPipe(NULL), _errorsCount(0)
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::~osPipeSocket
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osPipeSocket::~osPipeSocket()
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::channelType
// Description: Returns my channel type - a binary channel.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
osChannel::osChannelType osPipeSocket::channelType() const
{
    // This is a binary channel:
    return osChannel::OS_BINARY_CHANNEL;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::defaultReadOperationTimeOut
// Description: Returns the default read operation time out (measured in milliseconds).
// Author:      AMD Developer Tools Team
// Date:        1/7/2007
// ---------------------------------------------------------------------------
long osPipeSocket::defaultReadOperationTimeOut() const
{
    return OS_PIPE_SOCKET_DEFAULT_READ_TIMEOUT;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::defaultWriteOperationTimeOut
// Description: Returns the default write operation time out (measured in milliseconds).
// Author:      AMD Developer Tools Team
// Date:        1/7/2007
// ---------------------------------------------------------------------------
long osPipeSocket::defaultWriteOperationTimeOut() const
{
    return OS_PIPE_SOCKET_DEFAULT_WRITE_TIMEOUT;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::isOpen
// Description: Returns true iff the pipe is open.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocket::isOpen() const
{
    return (_isOpen && (_errorsCount == 0));
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::close
// Description: Terminates the pipe communication.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocket::close()
{
    bool retVal = false;

    BOOL incomingPipeClosed = FALSE;
    BOOL outgoingPipeClosed = FALSE;

    if (!_isDuringPipeConnectionWait)
    {
        // Close the incoming pipe:
        if (_incomingPipe != NULL)
        {
            incomingPipeClosed = ::CloseHandle(_incomingPipe);

            if (incomingPipeClosed == FALSE)
            {
                gtString debugMessage;
                debugMessage.appendFormattedString(OS_STR_pipeCannotClose, _socketName.asCharArray());
                GT_ASSERT_EX(false, debugMessage.asCharArray());
            }

            _incomingPipe = NULL;
        }

        // Close the outgoing pipe:
        if (_outgoingPipe != NULL)
        {
            outgoingPipeClosed = ::CloseHandle(_outgoingPipe);

            if (outgoingPipeClosed == FALSE)
            {
                gtString debugMessage;
                debugMessage.appendFormattedString(OS_STR_pipeCannotClose, _socketName.asCharArray());
                GT_ASSERT_EX(false, debugMessage.asCharArray());
            }

            _outgoingPipe = NULL;
        }

        if ((incomingPipeClosed != FALSE) && (outgoingPipeClosed != FALSE))
        {
            retVal = true;
        }
    }
    else
    {
        // If we are waiting for the connection, it is impossible to close the pipe
        // handle, and trying would cause the process debugger to get itsself stuck.
        // This usually happens when the debugged application crashes before the spy is
        // initialized, for example when it is missing a required dll.
        // Our only option in this case is to abandon the thread and simply move on with the
        // process run.
        _incomingPipe = NULL;
        _outgoingPipe = NULL;
        retVal = false;
    }

    // Mark that the pipe is closed:
    _isOpen = false;
    _errorsCount = 0;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::write
// Description: Write data into the outgoing pipe.
// Arguments: pDataBuffer - A buffer containing the data to be written.
//            dataSize - The data size.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocket::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // If the pipe is open and does not have errors:
    if (_isOpen && (_errorsCount == 0))
    {
        // Write the data into the outgoing pipe:
        DWORD numberOfBytesWritten = 0;
        BOOL rc1 = ::WriteFile(_outgoingPipe, (LPCVOID)pDataBuffer, (DWORD)dataSize, &numberOfBytesWritten, NULL);

        if ((rc1 != FALSE) && (numberOfBytesWritten == dataSize))
        {
            retVal = true;
        }

        // Testing section - leave commented out!
        /*
        gtString foo = "write: ";
        foo.appendFormattedString(L"to write: %d, wrote: %d", dataSize, rc);
        OS_OUTPUT_DEBUG_LOG(foo.asCharArray(), OS_INFO_SEVERITY);
        */
    }

    // If there was an error:
    if (!retVal)
    {
        // Mark that the pipe has an error:
        _errorsCount++;

        // Yaki - 11/2/2008:
        // We limit the amount of "Pipe read error" printouts, since some clients sent
        // us huge log files, containing millions of "Pipe read error" printouts.
        if (_errorsCount < OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeWriteError, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else if (_errorsCount == OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeWriteErrorLastMessage, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else
        {
            // Don't enable overflowing _errorsCount:
            _errorsCount = OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS + 10;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osPipeSocket::read
// Description: Read data from the incoming pipe.
// Arguments: pDataBuffer - A buffer that will contain the read data.
//            dataSize - The amount of data to be read.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
bool osPipeSocket::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // If the pipe is open and does not have errors:
    if (_isOpen && (_errorsCount == 0))
    {
        bool goOn = true;

        while (goOn)
        {
            // Read the data that is currently available in the pipe:
            DWORD readDataSize = 0;
            BOOL rc1 = ::ReadFile(_incomingPipe, (LPVOID)pDataBuffer, (DWORD)dataSize, &readDataSize, NULL);

            if (rc1 == FALSE)
            {
                // A read error occurred:
                gtString debugMessage;
                debugMessage.appendFormattedString(OS_STR_pipeReadError, _socketName.asCharArray());
                GT_ASSERT_EX(false, debugMessage.asCharArray());
                goOn = false;
            }
            else
            {
                // If the entire data was read:
                if (readDataSize == dataSize)
                {
                    retVal = true;
                    goOn = false;
                }
                else if (readDataSize < dataSize)
                {
                    // We still have data to read:

                    // Calculate the amount of data left to read:
                    dataSize -= readDataSize;
                    (char*)pDataBuffer += readDataSize;

                    // Let other threads with the same priority as mine run
                    // (maybe one of them is the thread that fills my input data)
                    ::Sleep(0);
                }
                else
                {
                    // Error - we read more data that we asked for:
                    gtString debugMessage;
                    debugMessage.appendFormattedString(OS_STR_pipeReadMoreDataThenAskedFor, _socketName.asCharArray());
                    GT_ASSERT_EX(false, debugMessage.asCharArray());
                    goOn = false;
                }
            }
        }
    }

    // If there was an error:
    if (!retVal)
    {
        // Mark that the pipe has an error:
        _errorsCount++;

        // Yaki - 11/2/2008:
        // We limit the amount of "Pipe read error" printouts, since some clients sent
        // us huge log files, containing millions of "Pipe read error" printouts.
        if (_errorsCount < OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadError, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else if (_errorsCount == OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadErrorLastMessage, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else
        {
            // Don't enable overflowing _errorsCount:
            _errorsCount = OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS + 10;
        }
    }

    // Testing section - leave commented out!
    /*
        gtString foo = "read: ";
        foo.appendFormattedString(L"to read: %d, read: %d", dataSize, rc);
        OS_OUTPUT_DEBUG_LOG(foo.asCharArray(), OS_INFO_SEVERITY);
    */

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::readAvailableData
//
// Description:
//   Reads and outputs the data that is currently available in the pipe.
//   If no data is available in the pipe for reading, waits _readOperationTimeOut
//   milliseconds until data is available.
//   If more data than bufferSize is available, reads only bufferSize bytes of data.
//
// Arguments: pDataBuffer - A buffer that will receive the data.
//            bufferSize - The buffer size.
//            amountOfDataRead - The amount of data actually read.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        10/2/2008
// ---------------------------------------------------------------------------
bool osPipeSocket::readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = false;

    DWORD amountOfDataReadAsDWORD = 0;

    // Read the data that is currently available in the pipe:
    BOOL rc1 = ::ReadFile(_incomingPipe, (LPVOID)pDataBuffer, (DWORD)bufferSize, &amountOfDataReadAsDWORD, NULL);
    GT_IF_WITH_ASSERT(rc1 != FALSE)
    {
        retVal = true;
    }

    amountOfDataRead = amountOfDataReadAsDWORD;

    // If there was an error:
    if (!retVal)
    {
        // Mark that the pipe has an error:
        _errorsCount++;

        // Yaki - 11/2/2008:
        // We limit the amount of "Pipe read error" printouts, since some clients sent
        // us huge log files, containing millions of "Pipe read error" printouts.
        if (_errorsCount < OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadError, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else if (_errorsCount == OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS)
        {
            gtString debugMessage;
            debugMessage.appendFormattedString(OS_STR_pipeReadErrorLastMessage, _socketName.asCharArray());
            GT_ASSERT_EX(false, debugMessage.asCharArray());
        }
        else
        {
            // Don't enable overflowing _errorsCount:
            _errorsCount = OS_PIPE_SOCKET_MAX_ERROR_PRINTOUTS + 10;
        }
    }

    // Testing section - leave commented out!
    /*
    gtString foo = "read: ";
    foo.appendFormattedString(L"to read: %d, read: %d", dataSize, rc);
    OS_OUTPUT_DEBUG_LOG(foo.asCharArray(), OS_INFO_SEVERITY);
    */

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::getPipeFIFOFilesPaths
// Description: Calculates and returns the names of the 2 FIFO files,
//              used for implementing this full duplex pipe socket.
// Arguments: clientToServer - The client to server pipe name.
//            serverToClient - The server to client pipe name.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/5/2007
// ---------------------------------------------------------------------------
void osPipeSocket::getPipeFIFOFilesPaths(gtString& clientToServer, gtString& serverToClient)
{
    // A Win32 pipe name is build out of the following format:
    // \\ServerName\pipe\PipeName where \\. is the local machine.
    const wchar_t* pipesNamePrefix = L"\\\\.\\pipe\\";

    serverToClient = pipesNamePrefix;
    serverToClient += _pipeName;
    serverToClient += L"-serverToClient";


    clientToServer = pipesNamePrefix;
    clientToServer += _pipeName;
    clientToServer += L"-clientToServer";
}

/////////////////////////////////////////////////////////////////////////
/// \brief Get server pipe socket OS name
///
/// \param filePath a server pipe socket OS name
///
/// \author:      AMD Developer Tools Team
/// \date:        02/03/2016
void osPipeSocket::getServerFilePath(gtString& filePath)
{
    osFilePath serverPipeFilePath(osFilePath::OS_TEMP_DIRECTORY);
    serverPipeFilePath.setFileName(_pipeName);
    serverPipeFilePath.setFileExtension(L"tmp1");
    filePath = serverPipeFilePath.asString();
}

/////////////////////////////////////////////////////////////////////////
/// \brief Get server pipe socket OS name
///
/// \param filePath a server pipe socket OS name
///
/// \author:      AMD Developer Tools Team
/// \date:        02/03/2016
void osPipeSocket::getClientFilePath(gtString& filePath)
{
    osFilePath clientPipeFilePath(osFilePath::OS_TEMP_DIRECTORY);
    clientPipeFilePath.setFileName(_pipeName);
    clientPipeFilePath.setFileExtension(L"tmp2");
    filePath = clientPipeFilePath.asString();
}


