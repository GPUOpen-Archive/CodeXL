//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCallStackFrame.cpp
///
//=====================================================================

//------------------------------ osCallStackFrame.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osCallStackFrame.h>
#include <AMDTOSWrappers/Include/osChannel.h>


// ---------------------------------------------------------------------------
// Name:        osCallStackFrame::osCallStackFrame
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
osCallStackFrame::osCallStackFrame()
    : _functionStartAddress((osInstructionPointer)NULL),
      _moduleStartAddress((osInstructionPointer)NULL),
      _instructionCounterAddress((osInstructionPointer)NULL),
      _isSpyFunction(false),
      _sourceCodeFileLineNumber(-1),
      _isKernelSourceCode(false)
{
}


// ---------------------------------------------------------------------------
// Name:        osCallStackFrame::~osCallStackFrame
// Description:
// Return Val:
// Author:      AMD Developer Tools Team
// Date:        9/10/2011
// ---------------------------------------------------------------------------
osCallStackFrame::~osCallStackFrame()
{
}

// ---------------------------------------------------------------------------
// Name:        osCallStackFrame::writeSelfIntoChannel
// Description: Writes self into channel
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool osCallStackFrame::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the module file path
    _moduleFilePath.writeSelfIntoChannel(ipcChannel);

    // Write the function start address
    ipcChannel << (gtUInt64)_functionStartAddress;

    // Write the module start address
    ipcChannel << (gtUInt64)_moduleStartAddress;

    // Write the instruction counter address
    ipcChannel << (gtUInt64)_instructionCounterAddress;

    // Write the function name
    ipcChannel << _functionName;

    // Write the "is spy function" flag
    ipcChannel << _isSpyFunction;

    // Write the source code file path
    bool retVal = _sourceCodeFilePath.writeSelfIntoChannel(ipcChannel);

    // Write the source code file line number
    ipcChannel << (gtInt32)_sourceCodeFileLineNumber;

    // Write the code type flag:
    ipcChannel << _isKernelSourceCode;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCallStackFrame::readSelfFromChannel
// Description: Reads self from channel
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool osCallStackFrame::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the module file path
    _moduleFilePath.readSelfFromChannel(ipcChannel);

    // Read the function start address
    gtUInt64 functionStartAddressAsUInt64 = 0;
    ipcChannel >> functionStartAddressAsUInt64;
    _functionStartAddress = (osInstructionPointer)functionStartAddressAsUInt64;

    // Read the module start address
    gtUInt64 moduleStartAddressAsUInt64 = 0;
    ipcChannel >> moduleStartAddressAsUInt64;
    _moduleStartAddress = (osInstructionPointer)moduleStartAddressAsUInt64;

    // Read the instruction counter address
    gtUInt64 instructionCounterAddressAsUInt64 = 0;
    ipcChannel >> instructionCounterAddressAsUInt64;
    _instructionCounterAddress = (osInstructionPointer)instructionCounterAddressAsUInt64;

    // Read the function name
    ipcChannel >> _functionName;

    // Read the "is spy function" flag
    ipcChannel >> _isSpyFunction;

    // Read the source code file path
    bool retVal = _sourceCodeFilePath.readSelfFromChannel(ipcChannel);

    // Read the source code file line number
    gtInt32 sourceCodeFileLineNumberAsInt32 = 0;
    ipcChannel >> sourceCodeFileLineNumberAsInt32;
    _sourceCodeFileLineNumber = (int)sourceCodeFileLineNumberAsInt32;

    // Read the code type flag:
    ipcChannel >> _isKernelSourceCode;

    return retVal;
}
