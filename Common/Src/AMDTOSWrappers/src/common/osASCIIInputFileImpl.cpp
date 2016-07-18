//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osASCIIInputFileImpl.cpp
///
//=====================================================================

//------------------------------ osASCIIInputFileImpl.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <common/osASCIIInputFileImpl.h>


// The size of the chunk that we read from the file:
#define OS_FILE_CHUNK_READ_SIZE 1024


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::osASCIIInputFileImpl
// Description: Default constructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osASCIIInputFileImpl::osASCIIInputFileImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::~osASCIIInputFileImpl
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osASCIIInputFileImpl::~osASCIIInputFileImpl()
{
    if (isOpened())
    {
        close();
    }
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::open
// Description: Opens the input file.
// Arguments:   path - The file path.
//              fileType - The file type.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::open(const osFilePath& path, osChannel::osChannelType fileType)
{
    bool retVal = false;

    // Get the iso open mode:
    std::ios_base::openmode isoOpenMode = fileOpenModeToIosOpenMode(osFile::OS_OPEN_TO_READ, fileType);

    // Open the output file stream:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    _inputFileStream.open(path.asString().asCharArray(), isoOpenMode);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    _inputFileStream.open(path.asString().asUTF8CharArray(), isoOpenMode);
#endif

    // Verify that the file was opened:
    if (_inputFileStream.is_open() && _inputFileStream.good())
    {
        if (fileType == osFile::OS_UNICODE_TEXT_CHANNEL)
        {
            // Read the unicode BOM:
            gtByte unicodeBom[2];
            gtSize_t amountOfCharsRead = 0;
            bool rcGetUnicodeBOM = read(unicodeBom, 2, amountOfCharsRead);
            GT_IF_WITH_ASSERT(rcGetUnicodeBOM)
            {
                char char1 = unicodeBom[0];
                char char2 = unicodeBom[1];
                GT_ASSERT((char1 == '\xFF') && (char2 == '\xFE'));
            }
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::close
// Description: Closes the file
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
void osASCIIInputFileImpl::close()
{
    if (_inputFileStream.is_open())
    {
        _inputFileStream.close();
    }
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::flush
// Description: Does nothing
// Author:      AMD Developer Tools Team
// Date:        16/8/2004
// ---------------------------------------------------------------------------
void osASCIIInputFileImpl::flush()
{
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::isOK
// Description: returns true iff no errors were recorded during the file operation.
// Author:      AMD Developer Tools Team
// Date:        1/12/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::isOK() const
{
    bool retVal = false;

    if ((((osASCIIInputFileImpl*)(this))->_inputFileStream).is_open())
    {
        retVal = _inputFileStream.good();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::write
// Description: Always fails - We do not allow writing into an input file.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::write(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    (void)(pDataBuffer); // unused
    (void)(dataSize); // unused
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::read
// Description: Reads data from the file.
// Arguments:   pDataBuffer - A buffer that will contain the read data.
//              bufferSize - The buffer size.
//              amountOfDataRead - Will get the amount of data read into the buffer.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = false;

    if (_inputFileStream.is_open())
    {
        // Read data from the file into the buffer:
        amountOfDataRead = 0;

        while (_inputFileStream.good() && (amountOfDataRead < bufferSize))
        {
            pDataBuffer[amountOfDataRead++] = (gtByte)_inputFileStream.get();
        }

        if (_inputFileStream.good())
        {
            retVal = true;
        }
        else
        {
            // We reached the end of the file:
            if (amountOfDataRead > 1)
            {
                // Remove the EOF char:
                amountOfDataRead--;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::readLine
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
bool osASCIIInputFileImpl::readLine(gtString& line)
{
    (void)(line); // unused
    bool retVal = false;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::readLine
// Description: Reads a line from the input file.
// Arguments:   line - Will get the read line.
// Return Val:  bool - Success / failure.
//              This method will fail when:
//   -          The stream is not an appropriate for this operation (binary file, output file, etc).
//   -          We reached the end of the file.
//              ASCII version
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::readLine(gtASCIIString& line)
{
    bool retVal = false;
    line.makeEmpty();

    // If there is data to be read in the file:
    if (_inputFileStream.is_open() && _inputFileStream.good())
    {
        retVal = true;

        // Contains true iff we still have data to read from the file.
        bool goOn = true;

        // Used to check for Windows style (CR+LF) line endings:
        char lastChar = 0x00;

        // Read the line in OS_FILE_CHUNK_READ_SIZE chunks from the input file:
        char pDataBuffer[OS_FILE_CHUNK_READ_SIZE + 1];

        while (goOn)
        {
            int i = 0;

            for (int j = 0; j < OS_FILE_CHUNK_READ_SIZE; j++)
            {
                // Get the current char from the string:
                pDataBuffer[i] = (char)_inputFileStream.get();

                // If we reached the end of the file:
                bool reachedEOF = !(_inputFileStream.good());

                if (reachedEOF)
                {
                    // If we didn't manage to read anything:
                    if ((i == 0) && (line.isEmpty()))
                    {
                        goOn = false;
                        retVal = false;
                        break;
                    }
                    else
                    {
                        // We read some content until reaching the EOF - exit the loops:
                        goOn = false;
                        break;
                    }
                }
                else if ((pDataBuffer[i] == 0x0A) || (pDataBuffer[i] == 0x0D))
                {
                    // We reached a line delimiter character:
                    // (0x0A - Line Feed, 0x0D - Carriage Return)

                    // If the last char was a CR and this one is a LF, ignore this char as it is the same line ending.
                    if ((lastChar == 0x0D) && (pDataBuffer[i] == 0x0A))
                    {
                        j--;
                        i--;
                    }
                    else
                    {
                        // The line is finished - exit the loops:
                        goOn = false;
                        lastChar = pDataBuffer[i];
                        break;
                    }
                }

                lastChar = pDataBuffer[i];

                i++;
            }

            if (0 < i)
            {
                // NULL terminate the read chunk:
                pDataBuffer[i] = (char)NULL;

                // Add it to the output string:
                line += pDataBuffer;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::isOpened
// Description: Returns true iff the file is opened.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::isOpened() const
{
    bool isStreamOpened = (((osASCIIInputFileImpl*)(this))->_inputFileStream).is_open();
    bool isStreamOk = _inputFileStream.good();
    return isStreamOpened && isStreamOk;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::seekCurrentPosition
// Description: Seeks the current read position.
// Arguments:   seekStartPosition - The position from which we give offset.
//              offset - The offset.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset)
{
    bool retVal = false;

    if (_inputFileStream.is_open())
    {
        // Translate the stream position to iostream terminology:
        std::ios_base::seekdir iosStreamPosition = streamPositionToIosSeekDir(seekStartPosition);

        // Seek the read position:
        _inputFileStream.seekg(offset, iosStreamPosition);

        // Check that all went ok:
        if (_inputFileStream.good())
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::currentPosition
// Description: Returns the current read position.
// Arguments:   positionReference - The reference position.
//              offset - Will get the current stream read position as an offset
//                       to the positionReference.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const
{
    bool retVal = false;

    if ((((osASCIIInputFileImpl*)(this))->_inputFileStream).is_open())
    {
        if (positionReference == osStream::OS_STREAM_BEGIN)
        {
            // Get the read position:
            std::streampos streamPosition = ((osASCIIInputFileImpl*)(this))->_inputFileStream.tellg();
            offset = (gtSize_t)streamPosition;
            retVal = true;
        }
        else
        {
            // TO_DO: Implement me.
            GT_ASSERT(false);
        }

        // Check that all went ok:
        if (_inputFileStream.good())
        {
            retVal = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osASCIIInputFileImpl::readIntoString
// Description: Read the stream into a string
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/11/2011
// ---------------------------------------------------------------------------
bool osASCIIInputFileImpl::readIntoString(gtString& str)
{
    (void)(str); // unused
    bool retVal = false;
    return retVal;
}
