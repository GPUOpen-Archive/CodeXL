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
// Name:        osInputFileImpl::osInputFileImpl
// Description: Default constructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osInputFileImpl::osInputFileImpl() : _pInputFileStream(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::~osInputFileImpl
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osInputFileImpl::~osInputFileImpl()
{
    if (isOpened())
    {
        close();
    }

    if (_pInputFileStream != NULL)
    {
        delete _pInputFileStream;
    }
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::flush
// Description: Does nothing
// Author:      AMD Developer Tools Team
// Date:        16/8/2004
// ---------------------------------------------------------------------------
void osInputFileImpl::flush()
{
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::write
// Description: Always fails - We do not allow writing into an input file.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::write(const gtByte* pDataBuffer, gtSize_t dataSize)
{
    (void)(pDataBuffer); // unused
    (void)(dataSize); // unused
    return false;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::readLine
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
bool osInputFileImpl::readLine(gtASCIIString& line)
{
    (void)(line); // unused
    return false;
}



// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::isOpened
// Description: Returns true iff the file is opened.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/8/2004
// ---------------------------------------------------------------------------
bool osInputFileImpl::isOpened() const
{
    bool retVal = false;

    if (_pInputFileStream != NULL)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osInputFileImpl::readIntoString
// Description: Read the stream into a string
// Arguments:   gtString& str
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/11/2011
// ---------------------------------------------------------------------------
bool osInputFileImpl::readIntoString(gtString& str)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(_pInputFileStream != NULL)
    {

        int rc = fseek(_pInputFileStream, 0, SEEK_END);

        if (rc == 0)
        {
            // Calculate the size of the stream:
            long size = ftell(_pInputFileStream);
            GT_IF_WITH_ASSERT(size > 3)
            {
                // Go to the beginning of the file:
                rc = fseek(_pInputFileStream, 0, SEEK_SET);

                if (rc == 0)
                {
                    // Read the unicode BOM:
                    gtUByte unicodeBom[2];
                    size_t freadRet = fread(unicodeBom, 2, 1, _pInputFileStream);
                    GT_ASSERT(freadRet != 0);
                    GT_ASSERT(unicodeBom[0] == (gtUByte)0xff && unicodeBom[1] == (gtUByte)0xfe);

                    // Get the amount of wide characters:
                    int wcharSize = sizeof(wchar_t);
                    int amountOfWChars = (size - 2) / wcharSize;

                    // Allocate a buffer for the data:
                    wchar_t* pData = new wchar_t[amountOfWChars + 1];


                    // Read the whole string as once:
                    freadRet = fread(pData, wcharSize, size - 2, _pInputFileStream);
                    GT_ASSERT(freadRet != 0);
                    pData[amountOfWChars] = 0;
                    str = pData;
                    delete[] pData;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}
