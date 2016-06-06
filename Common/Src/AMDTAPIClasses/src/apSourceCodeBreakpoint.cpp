//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSourceCodeBreakpoint.cpp
///
//==================================================================================

//------------------------------ apSourceCodeBreakpoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>

// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::apSourceCodeBreakpoint
// Description: Constructor
// Arguments:   const osFilePath& filePath
//              int lineNumber
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
apSourceCodeBreakpoint::apSourceCodeBreakpoint(const osFilePath& filePath, int lineNumber)
    : _filePath(filePath), _lineNumber(lineNumber)
{

}

apSourceCodeBreakpoint::apSourceCodeBreakpoint() : _filePath(L""), _lineNumber(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::~apSourceCodeBreakpoint
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
apSourceCodeBreakpoint::~apSourceCodeBreakpoint()
{

}

// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::compareToOther
// Description: Returns true iff otherBreakpoint is an identical breakpoint.
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
bool apSourceCodeBreakpoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a kernel source code breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT)
    {
        // Downcast it:
        const apSourceCodeBreakpoint& otherSourceBreakpoint = (const apSourceCodeBreakpoint&)otherBreakpoint;

        // Compare the file path and line number:
        if ((otherSourceBreakpoint.filePath() == _filePath) && (otherSourceBreakpoint.lineNumber() == lineNumber()))
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
osTransferableObjectType apSourceCodeBreakpoint::type() const
{
    return OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT;
}


// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
bool apSourceCodeBreakpoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apBreakPoint::writeSelfIntoChannel(ipcChannel);

    _filePath.writeSelfIntoChannel(ipcChannel);
    ipcChannel << (gtInt32)_lineNumber;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apSourceCodeBreakpoint::readSelfFromChannel
// Description: Reads my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/9/2011
// ---------------------------------------------------------------------------
bool apSourceCodeBreakpoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apBreakPoint::readSelfFromChannel(ipcChannel);

    _filePath.readSelfFromChannel(ipcChannel);
    gtInt32 lineNumberAsInt32 = -1;
    ipcChannel >> lineNumberAsInt32;
    _lineNumber = (int)lineNumberAsInt32;

    return retVal;
}