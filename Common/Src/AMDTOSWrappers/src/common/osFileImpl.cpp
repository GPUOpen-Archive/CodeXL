//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileImpl.cpp
///
//=====================================================================

//------------------------------ osFileImpl.cpp ------------------------------

// Local:
#include <common/osFileImpl.h>


// ---------------------------------------------------------------------------
// Name:        osFileImpl::osFileImpl
// Description: Default constructor.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osFileImpl::osFileImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osFileImpl::~osFileImpl
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
osFileImpl::~osFileImpl()
{
}


// ---------------------------------------------------------------------------
// Name:        osFileImpl::fileOpenModeToIosOpenMode
// Description: Translates the file open mode from osFile terminology to
//              iostream terminology.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
ios_base::openmode osFileImpl::fileOpenModeToIosOpenMode(osFile::osOpenMode openMode, osChannel::osChannelType fileType)
{
    // Translate the open mode to ios::open_mode:
    ios_base::openmode retVal = ios_base::in;

    if (openMode == osFile::OS_OPEN_TO_WRITE)
    {
        retVal = ios_base::out | ios_base::trunc;
    }
    else if (openMode == osFile::OS_OPEN_TO_APPEND)
    {
        retVal = ios_base::out | ios_base::app;
    }

    if ((fileType == osFile::OS_BINARY_CHANNEL) || (fileType == osFile::OS_UNICODE_TEXT_CHANNEL))
    {
        retVal = retVal | ios_base::binary;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osFileImpl::streamPositionToIosSeekDir
// Description:
//   Translates the stream reference position from osFile terminology to
//   iostream terminology.
// Author:      AMD Developer Tools Team
// Date:        3/8/2004
// ---------------------------------------------------------------------------
ios_base::seekdir osFileImpl::streamPositionToIosSeekDir(osFile::osStreamPosition streamPosition)
{
    // Current stream position:
    ios_base::seekdir retVal = ios_base::cur;

    if (streamPosition == osFile::OS_STREAM_BEGIN)
    {
        // Stream beginning:
        retVal = ios_base::beg;
    }
    else if (streamPosition == osFile::OS_STREAM_END)
    {
        // Stream end:
        retVal = ios_base::end;
    }

    return retVal;
}
