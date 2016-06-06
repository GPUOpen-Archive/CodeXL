//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osRawMemoryBuffer.cpp
///
//=====================================================================

//------------------------------ osRawMemoryBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>


// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::osRawMemoryBuffer
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osRawMemoryBuffer::osRawMemoryBuffer(): _pData(NULL), _dataSize(0)
{
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::osRawMemoryBuffer
// Description: Copy constructor
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osRawMemoryBuffer::osRawMemoryBuffer(const osRawMemoryBuffer& other)
{
    osRawMemoryBuffer::operator=(other);
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::~osRawMemoryBuffer
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osRawMemoryBuffer::~osRawMemoryBuffer()
{
    // Delete the buffer if it is assigned:
    clearBuffer();
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::operator=
// Description: Assignment operator
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osRawMemoryBuffer& osRawMemoryBuffer::operator=(const osRawMemoryBuffer& other)
{
    // Clear any other data we might have had:
    clearBuffer();

    // Get the size:
    _dataSize = other._dataSize;

    // Assign enough bytes to read the data:
    _pData = new gtByte[_dataSize];

    // Copy the data:
    ::memcpy((void*)_pData, (const void*)other._pData, _dataSize);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::type
// Description: Returns my transferable object type.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType osRawMemoryBuffer::type() const
{
    return OS_TOBJ_ID_RAW_MEMORY_BUFFER;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool osRawMemoryBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the buffer size:
    ipcChannel << (gtUInt64)_dataSize;

    // Write the buffer data:
    bool retVal = ipcChannel.write(_pData, _dataSize);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::readSelfFromChannel
// Description: Reads this class from a channel.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool osRawMemoryBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    // Clear any previous data:
    clearBuffer();

    // Read the buffer size:
    gtUInt64 dataSizeAsUInt64 = 0;
    ipcChannel >> dataSizeAsUInt64;
    _dataSize = (gtSize_t)dataSizeAsUInt64;

    // Assign a large enough buffer:
    _pData = new gtByte[_dataSize];

    // Read the buffer data:
    bool retVal = ipcChannel.read(_pData, _dataSize);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
osTransferableObject* osRawMemoryBuffer::clone() const
{
    osRawMemoryBuffer* pClone = new osRawMemoryBuffer(*this);

    return (osTransferableObject*)pClone;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::fromFile
// Description: Reads the contents of a file into the raw memory buffer
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool osRawMemoryBuffer::fromFile(const osFilePath& filePath)
{
    bool retVal = false;

    // Clear any previous data:
    clearBuffer();

    // Fail if the file doesn't exist:
    if (filePath.exists())
    {
        osFile fileToRead(filePath);

        // Open the file for reading:
        bool rcOpen = fileToRead.open(osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_READ);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            // Get the file size:
            unsigned long fileSize = 0;
            bool rcSize = fileToRead.getSize(fileSize);
            GT_IF_WITH_ASSERT(rcSize)
            {
                if (fileSize > 0)
                {
                    // Set the buffer size:
                    _dataSize = fileSize;

                    // Assign a buffer large enough to contain the data:
                    _pData = new gtByte[_dataSize];

                    // Read the file into the buffer:
                    gtSize_t readData = 0;
                    retVal = fileToRead.readAvailableData(_pData, _dataSize, readData);

                    // Make sure we read the whole file:
                    GT_ASSERT(readData == _dataSize);
                }
                else
                {
                    // The file is empty, just return true:
                    retVal = true;
                }
            }

            fileToRead.close();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::toFile
// Description: Writes the contents of the raw memory buffer into a file, deleting
//              any previous contents. If createIfNeeded is false, does not create
//              a file that didn't exist before.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool osRawMemoryBuffer::toFile(const osFilePath& filePath, bool createIfNeeded) const
{
    bool retVal = false;

    // Make sure the file exists or that we want to create it:
    if (createIfNeeded || filePath.exists())
    {
        osFile fileToWrite(filePath);

        // Open the file for writing:
        bool rcOpen = fileToWrite.open(osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            retVal = fileToWrite.write(_pData, _dataSize);

            fileToWrite.close();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::appendToFile
// Description: Writes the contents of the raw memory buffer into a file, appending
//              to any previous contents. If createIfNeeded is false, does not create
//              a file that didn't exist before.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool osRawMemoryBuffer::appendToFile(const osFilePath& filePath, bool createIfNeeded) const
{
    bool retVal = false;

    // Make sure the file exists or that we want to create it:
    if (createIfNeeded || filePath.exists())
    {
        osFile fileToAppend(filePath);

        // Open the file for appending:
        bool rcOpen = fileToAppend.open(osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_APPEND);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            retVal = fileToAppend.write(_pData, _dataSize);

            fileToAppend.close();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osRawMemoryBuffer::clearBuffer
// Description: Resets the buffer to empty state
// Author:      AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
void osRawMemoryBuffer::clearBuffer()
{
    if (_pData != NULL)
    {
        delete[] _pData;
        _pData = NULL;
    }

    _dataSize = 0;
}
