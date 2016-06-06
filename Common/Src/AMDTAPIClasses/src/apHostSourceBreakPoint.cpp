//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apHostSourceBreakPoint.cpp
///
//==================================================================================

//------------------------------ apHostSourceCodeBreakpoint.cpp ------------------------------
/// \file apHostSourceCodeBreakpoint.h
/// \brief Host code source (*.cpp, *.hpp etc) breakpoint defenition structure
/// \author Vadim Entov
/// \date 12/09/2015

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>



/////////////////////////////////////////////////////////////////////////////
/// \brief Additional constructor
/// \param osFilePath a path for targen breakpoint file
/// \param lineNum a breakpoint line number
///
/// \author Vadim Entov
/// \date 12/09/2015
apHostSourceCodeBreakpoint::apHostSourceCodeBreakpoint(const osFilePath& filePath, int lineNum)
    : m_FilePath(filePath), m_lineNumber(lineNum)
{

}

/////////////////////////////////////////////////////////////////////////////
/// \brief Destructor
/// \author Vadim Entov
/// \date 12/09/2015
apHostSourceCodeBreakpoint::~apHostSourceCodeBreakpoint()
{

}

/////////////////////////////////////////////////////////////////////////////
/// \brief Other breakpoint compare.
///
/// \param otherBreakpoint a reference to comaparing breakpoint class instance
///
/// \return In case self and target the same - true
///
/// \author Vadim Entov
/// \date 12/09/2015
bool apHostSourceCodeBreakpoint::compareToOther(const apBreakPoint& otherBreakpoint) const
{
    bool retVal = false;

    // If the other breakpoint is a kernel source code breakpoint:
    if (otherBreakpoint.type() == OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT)
    {
        // Downcast it:
        const apHostSourceCodeBreakpoint& otherHostSourceBreakpoint = (const apHostSourceCodeBreakpoint&)otherBreakpoint;

        // All source breakpoints must match the line number:
        if (otherHostSourceBreakpoint.lineNumber() == lineNumber())
        {
            // For unresolved breakpoints, compare the file path:
            if (otherHostSourceBreakpoint.filePath() == filePath())
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
/// \brief Get current class instance type
///
/// \return instance type
///
/// \author Vadim Entov
/// \date 12/09/2015
osTransferableObjectType apHostSourceCodeBreakpoint::type() const
{
    return OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT;
}


/////////////////////////////////////////////////////////////////////////////
/// \brief Serialization to channel
///
/// \param ipcChannel a reference to ICP channel instance
///
/// \return true on function call succedes or false
///
/// \author Vadim Entov
/// \date 12/09/2015
bool apHostSourceCodeBreakpoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apBreakPoint::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtInt32)m_lineNumber;
    m_FilePath.writeSelfIntoChannel(ipcChannel);

    return retVal;
}


/////////////////////////////////////////////////////////////////////////////
/// \brief DeSerialization from channel
///
/// \param ipcChannel a reference to ICP channel instance
///
/// \return true on function call succedes or false
///
/// \author Vadim Entov
/// \date 12/09/2015
bool apHostSourceCodeBreakpoint::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apBreakPoint::readSelfFromChannel(ipcChannel);

    gtInt32 lineNumberAsInt32 = -1;
    ipcChannel >> lineNumberAsInt32;
    m_lineNumber = (int)lineNumberAsInt32;

    m_FilePath.readSelfFromChannel(ipcChannel);

    return retVal;
}