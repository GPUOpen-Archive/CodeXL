//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelSourceCodeBreakpoint.cpp
///
//==================================================================================

//------------------------------ apKernelSourceCodeBreakpoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>



// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::apKernelSourceCodeBreakpoint
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        3/5/2012
// ---------------------------------------------------------------------------
apKernelSourceCodeBreakpoint::apKernelSourceCodeBreakpoint(const osFilePath& unresolvedProgramFilePath, int lineNum)
    : m_isUnresolved(true), m_isHSAILBreakpoint(false), m_unresolvedFilePath(unresolvedProgramFilePath), _programHandle(OA_CL_NULL_HANDLE), _lineNumber(lineNum)
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::apKernelSourceCodeBreakpoint
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
apKernelSourceCodeBreakpoint::apKernelSourceCodeBreakpoint(oaCLProgramHandle hProgram, int lineNum)
    : m_isUnresolved(false), m_isHSAILBreakpoint(false), _programHandle(hProgram), _lineNumber(lineNum)
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::~apKernelSourceCodeBreakpoint
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
apKernelSourceCodeBreakpoint::~apKernelSourceCodeBreakpoint()
{

}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::compareToOther
// Description: Returns true iff otherBreakpoint is an identical breakpoint.
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool apKernelSourceCodeBreakpoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a kernel source code breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
    {
        // Downcast it:
        const apKernelSourceCodeBreakpoint& otherKernelSourceBreakpoint = (const apKernelSourceCodeBreakpoint&)otherBreakpoint;

        // All source breakpoints must match the line number:
        if (otherKernelSourceBreakpoint.lineNumber() == lineNumber())
        {
            // Compare resolution status:
            if (otherKernelSourceBreakpoint.isUnresolved() && isUnresolved())
            {
                // For unresolved breakpoints, compare the file path:
                if (otherKernelSourceBreakpoint.unresolvedPath() == unresolvedPath())
                {
                    retVal = true;
                }
            }
            else if ((!otherKernelSourceBreakpoint.isUnresolved()) && (!isUnresolved()))
            {
                // Compare kernel source type:
                if ((!otherKernelSourceBreakpoint.isHSAILBreakpoint()) && (!isHSAILBreakpoint()))
                {
                    // Resolved OpenCL BP, compare the program handle:
                    if (otherKernelSourceBreakpoint.programHandle() == programHandle())
                    {
                        retVal = true;
                    }
                }
                else if (otherKernelSourceBreakpoint.isHSAILBreakpoint() && isHSAILBreakpoint())
                {
                    // Resolved HSAIL BP, compare kernel name:
                    if (otherKernelSourceBreakpoint.hsailKernelName() == hsailKernelName())
                    {
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apKernelSourceCodeBreakpoint::type() const
{
    return OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT;
}


// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::writeSelfIntoChannel
// Description: Writes my content into a channel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool apKernelSourceCodeBreakpoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apBreakPoint::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_programHandle;
    ipcChannel << (gtInt32)_lineNumber;
    ipcChannel << m_isUnresolved;
    m_unresolvedFilePath.writeSelfIntoChannel(ipcChannel);
    ipcChannel << m_isHSAILBreakpoint;
    ipcChannel << m_hsailKernelName;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apKernelSourceCodeBreakpoint::readSelfFromChannel
// Description: Reads my content from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool apKernelSourceCodeBreakpoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apBreakPoint::readSelfFromChannel(ipcChannel);

    gtUInt64 programHandleAsUInt64 = 0;
    ipcChannel >> programHandleAsUInt64;
    _programHandle = (oaCLProgramHandle)programHandleAsUInt64;
    gtInt32 lineNumberAsInt32 = -1;
    ipcChannel >> lineNumberAsInt32;
    _lineNumber = (int)lineNumberAsInt32;

    ipcChannel >> m_isUnresolved;
    m_unresolvedFilePath.readSelfFromChannel(ipcChannel);

    ipcChannel >> m_isHSAILBreakpoint;
    ipcChannel >> m_hsailKernelName;

    return retVal;
}