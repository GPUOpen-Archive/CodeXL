//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osInputFileImpl.cpp
///
//=====================================================================

//------------------------------ osInputFileImpl.cpp ------------------------------

// std:
#include <iosfwd>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <common/osInputFileImpl.h>


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::open
// Description: Opens the input file.
// Arguments:   path - The file path.
//              fileType - The file type.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::open(const osFilePath& path, osChannel::osChannelType fileType)
{
    bool retVal = false;

    // Check what should be the file open parameters:
    gtString fileOpenFlags = L"rb";

    if (fileType == osFile::OS_ASCII_TEXT_CHANNEL)
    {
        fileOpenFlags = L"rt";
    }

    // Open the file:
    _pInputFileStream = _wfopen(path.asString().asCharArray(), fileOpenFlags.asCharArray());
    GT_IF_WITH_ASSERT(NULL != _pInputFileStream)
    {
        if (fileType == osFile::OS_UNICODE_TEXT_CHANNEL)
        {
            // Read the unicode BOM:
            wchar_t unicodeBom;
            fread(&unicodeBom, 2, 1, _pInputFileStream);
            retVal = (unicodeBom == 0xfeff);
        }
        else
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::close
// Description: Closes the file
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
void osInputFileImpl::close()
{
    if (_pInputFileStream != NULL)
    {
        fclose(_pInputFileStream);
        _pInputFileStream = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::isOK
// Description: returns true iff no errors were recorded during the file operation.
// Author:      AMD Developer Tools Team
// Date:        1/12/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::isOK() const
{
    bool retVal = false;

    if (_pInputFileStream != NULL)
    {
        retVal = !feof(_pInputFileStream);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::read
// Description: Reads data from the file.
// Arguments:   pDataBuffer - A buffer that will contain the read data.
//              bufferSize - The buffer size.
//              amountOfDataRead - Will get the amount of data read into the buffer.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::read(gtByte* pDataBuffer, gtSize_t bufferSize, gtSize_t& amountOfDataRead)
{
    bool retVal = false;

    if (_pInputFileStream != NULL)
    {
        if (isOK())
        {
            amountOfDataRead = fread(pDataBuffer, 1, bufferSize, _pInputFileStream);
            retVal = true;

        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::readLine
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
bool osInputFileImpl::readLine(gtString& line)
{
    bool retVal = false;
    line.makeEmpty();

    if (_pInputFileStream != NULL)
    {
        // If there is data to be read in the file:
        if (isOK())
        {
            retVal = true;

            // Contains true iff we still have data to read from the file.
            bool goOn = true;

            // Used to check for Windows style (CR+LF) line endings:
            wchar_t lastChar = 0x00;

            // Read the line in OS_FILE_CHUNK_READ_SIZE chunks from the input file:
            wchar_t pDataBuffer[OS_FILE_CHUNK_READ_SIZE + 1];

            while (goOn)
            {
                int i = 0;

                for (int j = 0; j < OS_FILE_CHUNK_READ_SIZE; j++)
                {
                    // Get the current char from the string:
                    fread(&pDataBuffer[i], 2, 2, _pInputFileStream);

                    // If we reached the end of the file:
                    bool reachedEOF = !(isOK());

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
                    pDataBuffer[i] = (wchar_t)NULL;

                    // Add it to the output string:
                    line += pDataBuffer;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::seekCurrentPosition
// Description: Seeks the current read position.
// Arguments:   seekStartPosition - The position from which we give offset.
//              offset - The offset.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::seekCurrentPosition(osStream::osStreamPosition seekStartPosition, gtSize_t offset)
{
    GT_UNREFERENCED_PARAMETER(seekStartPosition);

    bool retVal = false;

    if (_pInputFileStream != NULL)
    {
        int rc = fseek(_pInputFileStream, (long)offset, 0);
        retVal = (rc == 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::currentPosition
// Description: Returns the current read position.
// Arguments:   positionReference - The reference position.
//              offset - Will get the current stream read position as an offset
//                       to the positionReference.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::currentPosition(osStream::osStreamPosition positionReference, gtSize_t& offset) const
{
    GT_UNREFERENCED_PARAMETER(positionReference);

    bool retVal = false;

    if ((((osInputFileImpl*)(this))->_pInputFileStream) != NULL)
    {
        fpos_t pos = 0;
        int rc  = fgetpos(_pInputFileStream, &pos);
        offset = (gtSize_t)pos;
        retVal = (rc == 0);
    }

    return retVal;
}
