//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osOutputFileImpl.cpp
///
//=====================================================================

//------------------------------ osOutputFileImpl.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <common/osOutputFileImpl.h>


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::osOutputFileImpl
// Description: Default constructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osOutputFileImpl::osOutputFileImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::~osOutputFileImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osOutputFileImpl::~osOutputFileImpl()
{
    if (isOpened())
    {
        close();
    }
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::open
// Description: Opens the output file.
// Arguments:   path - The file path.
//              fileType - The file type.
//              openMode - The file open mode.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::open(const osFilePath& path, osChannel::osChannelType fileType, osFile::osOpenMode openMode)
{
    bool retVal = false;

    // Get the iso open mode:
    std::ios_base::openmode isoOpenMode = fileOpenModeToIosOpenMode(openMode, fileType);

    // Open the output file stream:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    _outputFileStream.open(path.asString().asCharArray(), isoOpenMode);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    _outputFileStream.open(path.asString().asUTF8CharArray(), isoOpenMode);
#else
#error unknown system
#endif

    // Verify that the file was opened:
    if (_outputFileStream.is_open() && _outputFileStream.good())
    {
        if (fileType == osFile::OS_UNICODE_TEXT_CHANNEL)
        {
            // For unicode file, write the unicode prefix:
            _outputFileStream.write("\xFF\xFE", 2);
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::close
// Description: Closes the file.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
void osOutputFileImpl::close()
{
    if (_outputFileStream.is_open())
    {
        _outputFileStream.close();
    }
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::flush
// Description: Flush cached data into the file.
// Author:      AMD Developer Tools Team
// Date:        16/8/2004
// ---------------------------------------------------------------------------
void osOutputFileImpl::flush()
{
    if (_outputFileStream.is_open())
    {
        _outputFileStream.flush();
    }
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::isOK
// Description: returns true iff no errors were recorded during the file operation.
// Author:      AMD Developer Tools Team
// Date:        1/12/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::isOK() const
{
    bool retVal = false;

    if (((osOutputFileImpl*)(this))->_outputFileStream.is_open())
    {
        retVal = _outputFileStream.good();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::write
// Description: Writes data into the file.
// Arguments:   pDataBuffer - A buffer holding the data to be written.
//              dataSize - The size of the data to be written.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::write(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    bool retVal = false;

    if (_outputFileStream.is_open())
    {
        // Write the data into the file:
        _outputFileStream.write(pDataBuffer, dataSize);

        // Check that the writing went well:
        if (_outputFileStream.good())
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::read
// Description: Always fails (an output file does not support reading).
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    (void)(pDataBuffer); // unused
    (void)(bufferSize); // unused
    (void)(amountOfDataRead); // unused
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::readLine
// Description: Always fails (an output file does not support reading).
// Author:      AMD Developer Tools Team
// Date:        5/12/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::readLine(gtString& line)
{
    (void)(line); // unused
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::readIntoString
// Description: Always fails (an output file does not support reading).
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/11/2011
// ---------------------------------------------------------------------------
bool osOutputFileImpl::readIntoString(gtString& str)
{
    (void)(str); // unused
    return false;
}

// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::readLine
// Description: Always fails (an output file does not support reading).
// Author:      AMD Developer Tools Team
// Date:        5/12/2004
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::readLine
// Description: Always fails (an output file does not support reading).
//              ASCII version
// Arguments:   gtASCIIString& line
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osOutputFileImpl::readLine(gtASCIIString& line)
{
    (void)(line); // unused
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::isOpened
// Description: Return true iff the file is currently opened.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::isOpened() const
{
    bool isStreamOpened = ((osOutputFileImpl*)(this))->_outputFileStream.is_open();
    bool isStreamOk = _outputFileStream.good();
    return isStreamOpened && isStreamOk;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::seekCurrentPosition
// Description: Sets the current file output position (the position to which we
//              write to).
// Arguments:   seekStartPosition - The position from which we give offset.
//              offset - The offset.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset)
{
    bool retVal = false;

    if (_outputFileStream.is_open())
    {
        // Translate the stream position to iostream terminology:
        std::ios_base::seekdir iosStreamPosition = streamPositionToIosSeekDir(seekStartPosition);

        // Seek the write position:
        _outputFileStream.seekp(offset, iosStreamPosition);

        // Check that all went ok:
        if (_outputFileStream.good())
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osOutputFileImpl::currentPosition
// Description: Returns the current stream write position.
// Arguments:   positionReference - The reference position.
//              offset - Will get the current stream write position as an offset
//                       to the positionReference.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osOutputFileImpl::currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const
{
    bool retVal = false;

    if (((osOutputFileImpl*)(this))->_outputFileStream.is_open())
    {
        if (positionReference == osStream::OS_STREAM_BEGIN)
        {
            // Get the write position:
            std::streampos streamPosition = ((osOutputFileImpl*)(this))->_outputFileStream.tellp();
            offset = (gtSize_t)streamPosition;
            retVal = true;
        }
        else
        {
            // TO_DO: Implement me.
            GT_ASSERT(false);
        }

        // Check that all went ok:
        if (_outputFileStream.good())
        {
            retVal = true;
        }
    }

    return retVal;
}



