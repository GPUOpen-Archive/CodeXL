//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocket.cpp
///
//=====================================================================

//------------------------------ osPipeSocket.cpp ------------------------------

// POSIX:
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osSystemError.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeSocket.h>

// Default read / write timeouts (set to 3 minutes):
#define OS_PIPE_SOCKET_DEFAULT_READ_TIMEOUT 180000
#define OS_PIPE_SOCKET_DEFAULT_WRITE_TIMEOUT 180000

// Read interval (we divide the read timeout into small interval of this length):
#define OS_PIPE_SOCKET_READ_INTERVAL 1000

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
      _incomingPipe(-1), _outgoingPipe(-1), _errorsCount(0)
{
    // Set read and write operations time outs for this specific osChannel sub class:
    setReadOperationTimeOut(OS_PIPE_SOCKET_DEFAULT_READ_TIMEOUT);
    setWriteOperationTimeOut(OS_PIPE_SOCKET_DEFAULT_WRITE_TIMEOUT);
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
    if (_isOpen)
    {
        //          Bugs 8756, 8666
        // Make copies of the pipe file descriptor number for diagnostics.
        // The rc1 == 0 && rc2 == 0 assert is tripping, so need more context
        // Are we getting a split?  One pipe open and one closed on arrival?
        // At least one instance of (_incomingPipe == _outgoingPipe) has been
        // seen when _isOpen is true
        osPipeHandle    inPipeCopy  = _incomingPipe;
        int             incomingCloseTripCount = 0;
        osPipeHandle    outPipeCopy = _outgoingPipe;
        int             outgoingCloseTripCount = 0;

        // Close the incoming pipe:
        int rc1 = -1;

        if (0 > _incomingPipe)
        {
            rc1 = 0;
        }

        while (0 != rc1)
        {
            rc1 = ::close(_incomingPipe);
            incomingCloseTripCount++;

            if (0 == rc1)
            {
                _incomingPipe = -1;
                break;
            }
            else if (EINTR != errno)
            {
                break;
            }
        }

        int incomingCloseRC = rc1;
        error_t incomingErrno = errno;

        // Close the outgoing pipe:
        int rc2 = -1;

        if (0 > _outgoingPipe)
        {
            rc2 = 0;
        }

        while (0 != rc2)
        {
            rc2 = ::close(_outgoingPipe);
            outgoingCloseTripCount++;

            if (0 == rc2)
            {
                _outgoingPipe = -1;
            }
            else if (EINTR != errno)
            {
                break;
            }
        }

        int outgoingCloseRC = rc1;
        error_t outgoingErrno = errno;

        // Intuitive "quick" bandaid:
        // The hang appears to be related to EBADF coming back due to
        // re-closing a closed pipe.  Simply ignore them
        // This really needs further investigation.  This is a band-aid
        // done to try to get us to a level of stability which will enable
        // us to ship.      rgorton 10-April-2011
        if (incomingErrno == EBADF)
        {
            rc1 = 0;
        }

        if (outgoingErrno == EBADF)
        {
            rc2 = 0;
        }

        GT_IF_WITH_ASSERT((rc1 == 0) && (rc2 == 0))
        {
            _isOpen = false;
        }

        // Print a diagnotic message with this data:
        if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg;
            logMsg.appendFormattedString(L"Attempted to close pipe at address %p.\nIn: FD: %d; RC: %d; errno: %d; tries: %d\nOut: FD: %d; RC: %d; errno: %d; tries: %d\nresult: %ls",
                                         this,
                                         (int)inPipeCopy, incomingCloseRC, (int)incomingErrno, incomingCloseTripCount,
                                         (int)outPipeCopy, outgoingCloseRC, (int)outgoingErrno, outgoingCloseTripCount,
                                         _isOpen ? L"failure" : L"success");

            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
        }
    }
    else // !_isOpen
    {
        // Attempt to close the pipes anyway, ignoring the results:
        if ((0 <= _incomingPipe) && (0 == ::close(_incomingPipe)))
        {
            _incomingPipe = -1;
        }

        if ((0 <= _outgoingPipe) && (0 == ::close(_outgoingPipe)))
        {
            _outgoingPipe = -1;
        }
    }

    return !_isOpen;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::write
// Description: Write data into the outgoing pipe.
// Arguments: pDataBuffer - A buffer containing the data to be written.
//            dataSize - The data size.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// Implementation notes:
//   From write(P) man page we learn that when writing to a blocking FIFO,
//   on normal completion, write shall return the amount of bytes it was
//   requested to write. Therefore, we don't need a loop (like we do in read)
//   that tries to write more data in case that not all data was written.
// ---------------------------------------------------------------------------
bool osPipeSocket::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Testing section - leave commented out!
    /*
        gtString foo = "Write starts: ";
        foo.appendFormattedString(L"bytes to write: %d", dataSize);
        OS_OUTPUT_DEBUG_LOG(foo.asCharArray(), OS_INFO_SEVERITY);
    */

    // If the pipe is open and does not have errors:
    if (_isOpen && (0 <= _outgoingPipe) && (_errorsCount == 0))
    {
        // Calculate the write timeout:
        struct timeval writeTimeout;
        osTimeValFromMilliseconds(_writeOperationTimeOut, writeTimeout);

        // Build a pipe set that contains the outgoing pipe:
        fd_set outgoingPipesSet;
        FD_ZERO(&outgoingPipesSet);
        FD_SET(_outgoingPipe, &outgoingPipesSet);

        // Build a pipe set of pipes that will be tested for errors:
        fd_set exceptionPipeSet;
        FD_ZERO(&exceptionPipeSet);
        FD_SET(_outgoingPipe, &exceptionPipeSet);

        // Wait until we can write data into the pipe:
        int highestPipe = _outgoingPipe + 1;
        int rc1 = ::select(highestPipe, NULL, &outgoingPipesSet, &exceptionPipeSet, &writeTimeout);

        if (rc1 < 0)
        {
            // An error occurred:
            gtString errMsg = OS_STR_writeError;
            errMsg += OS_STR_osReportedErrorIs;
            gtString systemError;
            osGetLastSystemErrorAsString(systemError);
            errMsg += systemError;
            GT_ASSERT_EX(false, errMsg.asCharArray());

            _errorsCount++;
        }
        else if (rc1 == 0)
        {
            // The write timeout was reached:
            GT_ASSERT_EX(false, OS_STR_timeoutReached);
            _errorsCount++;
        }
        else
        {
            // If the pipe had an exception (error):
            bool pipeExceptionOccur = (FD_ISSET(_outgoingPipe, &exceptionPipeSet));

            if (pipeExceptionOccur)
            {
                // Write an appropriate error message:
                gtString errMsg = OS_STR_pipeException;
                errMsg += OS_STR_osReportedErrorIs;
                gtString systemError;
                osGetLastSystemErrorAsString(systemError);
                errMsg += systemError;
                GT_ASSERT_EX(false, errMsg.asCharArray());

                _errorsCount++;
            }
            else
            {
                // Write the data into the outgoing pipe:
                size_t rc = ::write(_outgoingPipe, pDataBuffer, dataSize);
                GT_IF_WITH_ASSERT(rc == dataSize)
                {
                    retVal = true;
                }
            }
        }
    }

    // Testing section - leave commented out!
    // OS_OUTPUT_DEBUG_LOG(L"Write ended", OS_INFO_SEVERITY);

    // If there was an error:
    if (!retVal)
    {
        gtString debugMessage;
        debugMessage.appendFormattedString(OS_STR_pipeWriteError, _socketName.asCharArray());
        GT_ASSERT_EX(false, debugMessage.asCharArray());
        _errorsCount++;
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
// Implementation notes:
//   Instead of waiting the entire _readOperationTimeOut, we wait in small time intervals
//   (max 1 sec). This enables clients to change the read time interval "on the fly"
//   as we are waiting for data to be available for reading. (See case 3050 for information
//   about the need for this feature).
// ---------------------------------------------------------------------------
bool osPipeSocket::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    gtSize_t readDataSize = 0;
    bool retVal = readDataFromPipe(pDataBuffer, dataSize, false, readDataSize);
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
    bool retVal = readDataFromPipe(pDataBuffer, bufferSize, true, amountOfDataRead);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocket::getPipeFIFOFilesPaths
// Description: Calculates and returns the names of the 2 FIFO files,
//              used for implementing this full duplex pipe socket.
// Arguments: clientToServer - The client to server FIFO file name.
//            serverToClient - The server to client FIFO file name.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/12/2006
// ---------------------------------------------------------------------------
void osPipeSocket::getPipeFIFOFilesPaths(gtString& clientToServer, gtString& serverToClient)
{
    osFilePath serverPipeFilePath(osFilePath::OS_TEMP_DIRECTORY);
    serverPipeFilePath.setFileName(_pipeName);
    serverPipeFilePath.setFileExtension(L"tmp1");
    serverToClient = serverPipeFilePath.asString();

    osFilePath clientPipeFilePath(osFilePath::OS_TEMP_DIRECTORY);
    clientPipeFilePath.setFileName(_pipeName);
    clientPipeFilePath.setFileExtension(L"tmp2");
    clientToServer = clientPipeFilePath.asString();
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



// ---------------------------------------------------------------------------
// Name:        osPipeSocket::readDataFromPipe
// Description:
//   Reads data from the pipe.
//   readOnlyAvailableData - true - Causes this function to implement readAvailableData().
//                         - false - Causes this function to implement read().
//
// Author:      AMD Developer Tools Team
// Date:        10/2/2008
// ---------------------------------------------------------------------------
bool osPipeSocket::readDataFromPipe(gtByte* pDataBuffer, gtSize_t bufferSize, bool readOnlyAvailableData, gtSize_t& readDataSize)
{
    bool retVal = false;

    // Will get the amount of data read so far:
    readDataSize = 0;

    // Testing section - leave commented out!
    /*
    gtString foo1 = "Read starts: ";
    foo1.appendFormattedString(L"bytes to read: %d", dataSize);
    OS_OUTPUT_DEBUG_LOG(foo1.asCharArray(), OS_INFO_SEVERITY);
    */

    // If the pipe is open and does not have errors:
    if (_isOpen && (_errorsCount == 0))
    {
        // Will hold the time left until the read operation timeout reaches:
        long timeLeftToTimeout = _readOperationTimeOut;

        // Will contain the current read interval timeout:
        struct timeval readIntervalTimeout;

        // Loop until we get all required data:
        bool goOn = true;

        while (goOn)
        {
            // Calculate current read interval timeout (we allow maximal interval of OS_PIPE_SOCKET_READ_INTERVAL):
            if (OS_PIPE_SOCKET_READ_INTERVAL < timeLeftToTimeout)
            {
                osTimeValFromMilliseconds(OS_PIPE_SOCKET_READ_INTERVAL, readIntervalTimeout);
            }
            else
            {
                osTimeValFromMilliseconds(timeLeftToTimeout, readIntervalTimeout);
            }

            // Testing section - leave commented out!
            // OS_OUTPUT_DEBUG_LOG(L"Doing select", OS_INFO_SEVERITY);

            // Build a pipe set that contains the incoming pipe:
            fd_set incomingPipesSet;
            FD_ZERO(&incomingPipesSet);
            FD_SET(_incomingPipe, &incomingPipesSet);

            // Build a pipe set of pipes that will be tested for errors:
            fd_set exceptionPipeSet;
            FD_ZERO(&exceptionPipeSet);
            FD_SET(_incomingPipe, &exceptionPipeSet);

            // Wait until we have data to read:
            int highestPipe = _incomingPipe + 1;
            int rc1 = ::select(highestPipe, &incomingPipesSet, NULL, &exceptionPipeSet, &readIntervalTimeout);

            if (0 < rc1)
            {
                // If the pipe had an exception (error):
                bool pipeExceptionOccur = (FD_ISSET(_incomingPipe, &exceptionPipeSet));

                if (pipeExceptionOccur)
                {
                    // Build an appropriate error message:
                    gtString errMsg = OS_STR_pipeException;
                    errMsg += OS_STR_osReportedErrorIs;
                    gtString systemError;
                    osGetLastSystemErrorAsString(systemError);
                    errMsg += systemError;
                    GT_ASSERT_EX(false, errMsg.asCharArray());

                    _errorsCount++;
                    goOn = false;
                }
                else
                {
                    // We have data available for reading:
                    // Read the available data:
                    size_t rc2 = ::read(_incomingPipe, pDataBuffer, bufferSize);

                    // Testing section - leave commented out!
                    /*
                    gtString foo2 = "Read bytes: ";
                    foo2.appendFormattedString(L"%d", rc2);
                    OS_OUTPUT_DEBUG_LOG(foo2.asCharArray(), OS_INFO_SEVERITY);
                    */

                    // If there was no data to read.
                    if (rc2 == 0)
                    {
                        // This should not happened, since select() told us that we do have data to read.
                        // Getting here usually means that the pipe was broken.
                        gtString dbgMessage;
                        dbgMessage.appendFormattedString(OS_STR_readError, _socketName.asCharArray());
                        GT_ASSERT_EX(false, dbgMessage.asCharArray());
                        _errorsCount++;
                        goOn = false;
                    }

                    // Add the read data to the read data buffer:
                    readDataSize += rc2;
                    pDataBuffer += rc2;

                    // If we read all required data:
                    if (bufferSize == readDataSize)
                    {
                        goOn = false;
                        retVal = true;

                        // Testing section - leave commented out!
                        // OS_OUTPUT_DEBUG_LOG(L"Read all required data", OS_INFO_SEVERITY);
                    }
                    else if (bufferSize < readDataSize)
                    {
                        // We read too much data (this code should not be reached):
                        gtString dbgMessage;
                        dbgMessage.appendFormattedString(OS_STR_readError, _socketName.asCharArray());
                        GT_ASSERT_EX(false, dbgMessage.asCharArray());
                        _errorsCount++;
                        goOn = false;
                    }
                    else
                    {
                        // If we were asked to read only the first available data:
                        if (readOnlyAvailableData)
                        {
                            goOn = false;
                            retVal = true;
                        }
                        else
                        {
                            // Testing section - leave commented out!
                            // OS_OUTPUT_DEBUG_LOG(L"Still have data to read - doing another read iteration", OS_INFO_SEVERITY);
                        }
                    }
                }
            }
            else if (rc1 == 0)
            {
                // Timeout for current interval elapsed, but there is no data in the pipe for reading:

                // Calculate the time left for read operation timeout:
                timeLeftToTimeout -= OS_PIPE_SOCKET_READ_INTERVAL;

                // If we reached the read operation timeout:
                if (timeLeftToTimeout <= 0)
                {
                    GT_ASSERT_EX(false, OS_STR_timeoutReached);
                    goOn = false;
                }
                else
                {
                    // If someone reduced the read operation timeout while we are waiting
                    // for data (see "implementation notes" at the header of this function):
                    if (_readOperationTimeOut < timeLeftToTimeout)
                    {
                        timeLeftToTimeout = _readOperationTimeOut;

                        // Testing section - leave commented out!
                        // OS_OUTPUT_DEBUG_LOG(L"Reducing timeout", OS_INFO_SEVERITY);
                    }

                    // Do another wait interval ...
                }
            }
            else if (rc1 == -1)
            {
                // An error occurred:

                // If select() was interrupted by a signal:
                osSystemErrorCode lastSystemError = osGetLastSystemError();

                if (lastSystemError == EINTR)
                {
                    // Continue waiting for the pipe:
                    goOn = true;
                }
                else
                {
                    // Build an appropriate error message:
                    gtString dbgMessage;
                    dbgMessage.appendFormattedString(OS_STR_readError, _socketName.asCharArray());
                    GT_ASSERT_EX(false, dbgMessage.asCharArray());

                    dbgMessage += OS_STR_osReportedErrorIs;
                    gtString systemError;
                    osGetLastSystemErrorAsString(systemError);
                    dbgMessage += systemError;
                    GT_ASSERT_EX(false, dbgMessage.asCharArray());

                    goOn = false;
                }
            }
        }
    }

    // Testing section - leave commented out!
    // OS_OUTPUT_DEBUG_LOG(L"Read ended", OS_INFO_SEVERITY);

    // If there was an error:
    if (!retVal)
    {
        // Mark that the pipe has an error:
        _errorsCount++;

        // Yaki - 4/11/2007:
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

    return retVal;
}

