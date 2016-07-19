//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFile.cpp
///
//=====================================================================

//------------------------------ osFile.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// POSIX:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    // This header file is not found on Windows machines for some reason:
    #include <unistd.h>
#endif

// Local:
#include <common/osOutputFileImpl.h>
#include <common/osInputFileImpl.h>
#include <common/osASCIIInputFileImpl.h>
#include <AMDTOSWrappers/Include/osFile.h>

// ---------------------------------------------------------------------------
// Name:        osFile::osFile
// Description: Default constructor.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
osFile::osFile()
    : _pFileImpl(NULL), _fileType(osChannel::OS_BINARY_CHANNEL)
{
}


// ---------------------------------------------------------------------------
// Name:        osFile::osFile
// Description: Constructor
// Arguments:   path - The file path.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
osFile::osFile(const osFilePath& path)
    : _pFileImpl(NULL), _fileType(osChannel::OS_BINARY_CHANNEL), _filePath(path)
{
}


// ---------------------------------------------------------------------------
// Name:        osFile::~osFile
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
osFile::~osFile()
{
    // If the file is opened - close it:
    if (osFile::isOpened())
    {
        osFile::close();
    }

    delete _pFileImpl;
}


// ---------------------------------------------------------------------------
// Name:        osFile::open
// Description: Opens / Creates a file.
// Arguments:   The path of the file to be opened / created.
//              fileType - The file type.
//              openMode - The file open mode.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool osFile::open(const osFilePath& path, osChannel::osChannelType fileType, osOpenMode openMode)
{
    bool retVal = false;

    // Verify that the file is not already opened:
    if (!osFile::isOpened())
    {
        // Save the file type:
        _fileType = fileType;

        // Create the appropriate file implementation:
        if (openMode == osFile::OS_OPEN_TO_READ)
        {
            // Create an input file implementation:
            if (fileType == OS_UNICODE_TEXT_CHANNEL)
            {
                // Open an unicode file:
                osInputFileImpl* pInputFile = new osInputFileImpl;

                // Set my file implementation:
                _pFileImpl = pInputFile;

                // Open the input file:
                retVal = pInputFile->open(path, fileType);
            }
            else
            {
                // Open an ASCII file:
                osASCIIInputFileImpl* pInputFile = new osASCIIInputFileImpl;

                // Set my file implementation:
                _pFileImpl = pInputFile;

                // Open the input file:
                retVal = pInputFile->open(path, fileType);
            }
        }
        else
        {
            // Create an output file implementation:
            osOutputFileImpl* pOutputFile = new osOutputFileImpl;

            // Open the input file:
            retVal = pOutputFile->open(path, fileType, openMode);
            _pFileImpl = pOutputFile;
        }
    }

    // If the file was opened successfully - update the file path member:
    if (retVal)
    {
        _filePath = path;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::open
// Description: Opens / Creates a file. Uses this class set file path.
// Arguments:   fileType - The file type.
//              openMode - The file open mode.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
bool osFile::open(osChannelType fileType, osOpenMode openMode)
{
    bool retVal = open(_filePath, fileType, openMode);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::close
// Description: Close the file.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
void osFile::close()
{
    if (osFile::isOpened())
    {
        // Close the stream:
        _pFileImpl->close();

        // Delete it:
        delete _pFileImpl;
        _pFileImpl = NULL;

        // Reset the file type:
        _fileType = osChannel::OS_BINARY_CHANNEL;
    }
}

// ---------------------------------------------------------------------------
// Name:        osFile::flush
// Description: Flushed cached data into the file.
// Author:      AMD Developer Tools Team
// Date:        16/8/2004
// ---------------------------------------------------------------------------
void osFile::flush()
{
    if (osFile::isOpened())
    {
        _pFileImpl->flush();
    }
}


// ---------------------------------------------------------------------------
// Name:        osFile::isOK
// Description: Returns true iff the file is opened and no errors were recorded
//              during its operation.
// Author:      AMD Developer Tools Team
// Date:        1/12/2004
// ---------------------------------------------------------------------------
bool osFile::isOK() const
{
    bool retVal = false;

    if (osFile::isOpened())
    {
        retVal = _pFileImpl->isOK();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::setPath
// Description:
//   Sets the file path.
//   Notice: You cannot change the path of an opened file.
// Arguments:   path - The new file path.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
bool osFile::setPath(const osFilePath& path)
{
    bool retVal = false;

    // Verify that we are not trying to change the path of an open file:
    if (!isOpened())
    {
        _filePath = path;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::exists
// Description: Returns true iff the file exists.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
bool osFile::exists() const
{
    bool retVal = false;

    // If the file is opened - it exists:
    if (isOpened())
    {
        retVal = true;
    }
    else
    {
        retVal = _filePath.exists();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::channelType
// Description: Return my channel type
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
osChannel::osChannelType osFile::channelType() const
{
    return _fileType;
}


// ---------------------------------------------------------------------------
// Name:        osFile::readLine
// Description:
//   Reads a line from the input file.
// Arguments: line - Will get the read line.
// Return Val:
//  bool - Success / failure.
//   This method will fail when:
//   - The stream is not an appropriate for this operation (binary file, output file, etc).
//   - We reached the end of the file.
//
// Author:      AMD Developer Tools Team
// Date:        5/12/2004
// ---------------------------------------------------------------------------
bool osFile::readLine(gtString& line)
{
    bool retVal = false;

    // Sanity test:
    if (_pFileImpl && (_fileType != osChannel::OS_BINARY_CHANNEL))
    {
        // Read a line from the stream:
        retVal = _pFileImpl->readLine(line);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFile::readLine
// Description: Same as the above readLine - ASCII version
// Arguments:   line - Will get the read line.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osFile::readLine(gtASCIIString& line)
{
    bool retVal = false;

    // Sanity test:
    if (_pFileImpl && (_fileType != osChannel::OS_BINARY_CHANNEL))
    {
        // Read a line from the stream:
        retVal = _pFileImpl->readLine(line);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFile::readIntoString
// Description: Reads a text file content into a string.
// Arguments: string - A string that will receive the text file content.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool osFile::readIntoString(gtString& string)
{
    bool retVal = false;

    // Sanity test:
    if (_pFileImpl && (_fileType != osChannel::OS_BINARY_CHANNEL))
    {
        // Read a line from the stream:
        retVal = _pFileImpl->readIntoString(string);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::readIntoString
// Description: Reads a text file content into a string.
// Arguments: string - A string that will receive the text file content.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2005
// ---------------------------------------------------------------------------
bool osFile::readIntoString(gtASCIIString& string)
{
    // Read the first file line into the output string:
    string.makeEmpty();
    bool retVal = readLine(string);

    // If we managed to read the first file line:
    if (retVal)
    {
        // Iterate the file lines:
        gtASCIIString currentLine;
        bool goOn = true;

        while (goOn)
        {
            // Read the current file line:
            currentLine.makeEmpty();
            goOn = readLine(currentLine);

            // If we managed to read another line from the file:
            if (goOn)
            {
                // Add the line to the output string:
                string += "\n";
                string += currentLine;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::write
// Description: Write into the file stream.
// Arguments:   pDataBuffer - A buffer holding the data to be written.
//              dataSize - The size of the data to be written
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool osFile::writeImpl(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    if (_pFileImpl)
    {
        // Write the data into the stream:
        retVal = _pFileImpl->write(pDataBuffer, dataSize);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::writeString
// Description: Write a string into the file stream
// Arguments:   const gtString& str - string holding the data to be written
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
bool osFile::writeStringImpl(const gtString& str)
{
    bool retVal = false;

    if (_pFileImpl)
    {
        // NOCITE: Wide character string can only be written to a unicode / binary files:
        if (_fileType == osChannel::OS_UNICODE_TEXT_CHANNEL)
        {
            if (!str.isEmpty())
            {
                // Write the data into the stream:
                retVal = _pFileImpl->write((const gtByte*)str.asCharArray(), str.lengthInBytes());
            }
        }
        else if (_fileType == osChannel::OS_ASCII_TEXT_CHANNEL)
        {
            if (!str.isEmpty())
            {
                // Write the data into the stream:
                retVal = _pFileImpl->write((const gtByte*)str.asASCIICharArray(), str.length());
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFile::read
// Description: Reads a defined amount of data from the stream.
// Arguments:   pDataBuffer - A buffer that will receive the data.
//              dataSize - The data size.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
bool osFile::readImpl(gtByte* pDataBuffer, gtSize_t dataSize)
{
    gtSize_t amountOfDataRead;
    return readAvailableData(pDataBuffer, dataSize, amountOfDataRead);
}


// ---------------------------------------------------------------------------
// Name:        osFile::readString
// Description:
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/9/2010
// ---------------------------------------------------------------------------
bool osFile::readStringImpl(gtString& str)
{
    (void)(str); // unused
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osFile::read
// Description: Reads as much data as possible from the file.
// Arguments:   pDataBuffer - A buffer that will receive the data.
//              bufferSize - The buffer size.
//              amountOfDataRead - The amount of data actually read from the file.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/8/2005
// ---------------------------------------------------------------------------
bool osFile::readAvailableDataImpl(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = false;

    if (_pFileImpl)
    {
        // Read the data from the stream:
        retVal = _pFileImpl->read(pDataBuffer, bufferSize, amountOfDataRead);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::isOpened
// Description: Returns true iff the file is currently opened.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool osFile::isOpened() const
{
    bool retVal = false;

    if (_pFileImpl)
    {
        retVal = _pFileImpl->isOpened();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::seekCurrentPosition
// Description: Seeks the current file position.
// Arguments:   seekStartPosition - The position from which we reference.
//              offset - The offset from seekStartPosition.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osFile::seekCurrentPosition(osStreamPosition seekStartPosition, gtSize_t offset)
{
    bool retVal = false;

    if (_pFileImpl)
    {
        retVal = _pFileImpl->seekCurrentPosition(seekStartPosition, offset);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFile::currentPosition
// Description: Returns the current file position.
// Arguments:   positionReference - The position from which we reference.
//              offset - Will get the file offset from positionReference.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osFile::currentPosition(osStreamPosition positionReference, gtSize_t& offset) const
{
    bool retVal = false;

    if (_pFileImpl)
    {
        retVal = _pFileImpl->currentPosition(positionReference, offset);
    }

    return retVal;
}


