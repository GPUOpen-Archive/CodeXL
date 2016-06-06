//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osRawMemoryStream.cpp
///
//=====================================================================

//------------------------------ osRawMemoryStream.cpp ------------------------------

// Standard C:
#include <memory.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIAllocationFailureObserver.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::osRawMemoryStream
// Description: Constructor
// Arguments:   initialBufferSize - The raw memory buffer initial size
//                                  (amount of gtByte elements).
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
osRawMemoryStream::osRawMemoryStream(size_t initialBufferSize, bool safeAccessEnabled)
    : _pRawMemoryBuffer(NULL), _rawMemoryBufferSize(0),
      _currentWritePosition(0), _currentReadPosition(0),
      _pIAllocationFailureObserver(NULL), _safeAccessEnabled(safeAccessEnabled)
{
    // Resize the vector to the requested size:
    bool rc = resizeBuffer(initialBufferSize);
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::~osRawMemoryStream
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
osRawMemoryStream::~osRawMemoryStream()
{
    // We cache this value so it doesn't changed whiled we're writing:
    bool enteredCS = _safeAccessEnabled;

    if (enteredCS)
    {
        _writeAccessCS.enter();
    }

    // Delete the raw memory vector:
    delete[] _pRawMemoryBuffer;
    _pRawMemoryBuffer = NULL;

    if (enteredCS)
    {
        _writeAccessCS.leave();
    }
}


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::clear
// Description: Makes the stream empty.
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
void osRawMemoryStream::clear()
{
    // We cache this value so it doesn't changed whiled we're writing:
    bool enteredCS = _safeAccessEnabled;

    if (enteredCS)
    {
        _writeAccessCS.enter();
    }

    _currentWritePosition = 0;
    _currentReadPosition = 0;

    if (enteredCS)
    {
        _writeAccessCS.leave();
    }
}


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::registerAllocationFailureObserver
// Description: Registers an observer that will be notified when this class
//              fails to allocate the memory needed for its operation.
// Arguments: pIAllocationFailureObserver - The observer to be registered or
//                                          NULL to unregister the observer.
// Author:      AMD Developer Tools Team
// Date:        31/1/2009
// ---------------------------------------------------------------------------
void osRawMemoryStream::registerAllocationFailureObserver(gtIAllocationFailureObserver* pIAllocationFailureObserver)
{
    _pIAllocationFailureObserver = pIAllocationFailureObserver;
}


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::resizeBuffer
// Description: Sets the raw memory buffer size.
// Arguments:   newSize - The new raw memory buffer size.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
bool osRawMemoryStream::resizeBuffer(size_t newSize)
{
    bool retVal = false;

    if (newSize == _rawMemoryBufferSize)
    {
        // Nothing to be done:
        retVal = true;
    }
    else
    {
        // Calculate the smaller memory size (new or old):
        size_t smallerSize = (newSize < _rawMemoryBufferSize) ? newSize : _rawMemoryBufferSize;

        // Allocate a new vector:
        gtByte* pNewVector = NULL;

        try
        {
            pNewVector = new gtByte[newSize];
        }
        catch (...)
        {
            // Memory allocation failed:
            pNewVector = NULL;

            // If we have an allocation failure obsever - notify it:
            if (_pIAllocationFailureObserver != NULL)
            {
                _pIAllocationFailureObserver->onAllocationFailure();
            }
        }

        // If allocation succeeded:
        GT_IF_WITH_ASSERT_EX((pNewVector != NULL), OS_STR_FailedToAllocateMemory)
        {
            if (0 < smallerSize)
            {
                // Copy the old vector values to the new one:
                memcpy(pNewVector, _pRawMemoryBuffer, smallerSize);
            }

            // Delete the old vector and make the new vector the active vector:
            delete[] _pRawMemoryBuffer;
            _pRawMemoryBuffer = pNewVector;

            // Save its size:
            _rawMemoryBufferSize = newSize;

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::channelType
// Description: Return my channel type - binary channel.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
osChannel::osChannelType osRawMemoryStream::channelType() const
{
    return osChannel::OS_BINARY_CHANNEL;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::write
// Description: Writes a raw memory chunk into the stream.
// Arguments:   pDataBuffer - Pointer to the raw memory chunk.
//              dataSize - The chunk size.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
bool osRawMemoryStream::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool rc = true;

    // We cache this value so it doesn't changed whiled we're writing:
    bool enteredCS = _safeAccessEnabled;

    if (enteredCS)
    {
        _writeAccessCS.enter();
    }

    // If we need to resize the raw memory buffer:
    size_t requiredBufferSize = dataSize + _currentWritePosition;

    if (_rawMemoryBufferSize < requiredBufferSize)
    {
        // Calculate the new buffer size:
        size_t newSize = _rawMemoryBufferSize * 2;

        if (newSize <= requiredBufferSize)
        {
            newSize = newSize + dataSize + 1024;
        }

        // Resize the buffer to the new size:
        rc = resizeBuffer(newSize);
    }

    if (rc)
    {
        // Copy the input memory chunk into the raw memory buffer:
        gtByte* writeDestination = _pRawMemoryBuffer + _currentWritePosition;

        if (dataSize == 4)
        {
            *((gtUInt32*)writeDestination) = *((gtUInt32*)pDataBuffer);
        }
        else
        {
            memcpy(writeDestination, pDataBuffer, dataSize);
        }

        // Update the current stream read / write position:
        _currentWritePosition += dataSize;
    }

    if (enteredCS)
    {
        _writeAccessCS.leave();
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::read
// Description: Reads a raw memory chunk from the stream.
// Arguments:   pDataBuffer - Will get the read memory chunk.
//              dataSize - Amount of memory to read.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2004
// ---------------------------------------------------------------------------
bool osRawMemoryStream::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    // Verify that we have enough data:
    if ((_currentReadPosition + dataSize) <= _currentWritePosition)
    {
        // Read the memory from the stream:
        gtByte* readFromDestination = _pRawMemoryBuffer + _currentReadPosition;
        memcpy(pDataBuffer, readFromDestination, dataSize);

        // Advance _currentPosition location:
        _currentReadPosition += dataSize;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryStream::readAvailableData
//
// Description:
//   Reads and outputs the data that is currently available in the memory stream.
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
bool osRawMemoryStream::readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = true;
    amountOfDataRead = 0;

    // If we have data to output:
    if (_currentReadPosition < _currentWritePosition)
    {
        // Calculate the amount of data to be read:
        amountOfDataRead = _currentWritePosition - _currentReadPosition;

        if (bufferSize < amountOfDataRead)
        {
            amountOfDataRead = bufferSize;
        }

        // Read the memory from the stream:
        gtByte* readFromDestination = _pRawMemoryBuffer + _currentReadPosition;
        memcpy(pDataBuffer, readFromDestination, amountOfDataRead);

        // Advance _currentPosition location:
        _currentReadPosition += amountOfDataRead;
    }

    return retVal;
}


