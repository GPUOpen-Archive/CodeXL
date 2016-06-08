//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStream.cpp
///
//=====================================================================

//------------------------------ osStream.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osStream.h>


// ---------------------------------------------------------------------------
// Name:        osStream::seekCurrentPosition
// Description: Seeks the current position in this stream to a given offset.
//              The offset is given from the current stream position.
// Arguments:   long offset
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2004
// ---------------------------------------------------------------------------
bool osStream::seekCurrentPosition(gtSize_t offset)
{
    return seekCurrentPosition(OS_STREAM_CURRENT_POSITION, offset);
}


// ---------------------------------------------------------------------------
// Name:        osStream::currentPosition
// Description: Returns the stream current position, relative to the
//              stream begin position.
// Return Val:  long - The stream current position (relative to the stream beginning).
//                     or 0 in case of failure.
// Author:      AMD Developer Tools Team
// Date:        3/6/2004
// ---------------------------------------------------------------------------
gtSize_t osStream::currentPosition() const
{
    gtSize_t offset = 0;

    // Get the stream current position (relative to the stream beginning):
    bool rc = currentPosition(OS_STREAM_BEGIN, offset);

    // In case of failure - we return 0:
    if (!rc)
    {
        offset = 0;
    }

    return offset;
}

