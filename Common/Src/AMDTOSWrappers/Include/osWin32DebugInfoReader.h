//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32DebugInfoReader.h
///
//=====================================================================

//------------------------------ osWin32DebugInfoReader.h ------------------------------

#ifndef __OSWIN32DEBUGINFOREADER
#define __OSWIN32DEBUGINFOREADER

// Pre-decelerations:
class osFilePath;
class osCallStackFrame;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API osWin32DebugInfoReader
// General Description:
//   Enables reading a process debug information (if exists).
// Author:      AMD Developer Tools Team
// Creation Date:        9/05/2005
// ----------------------------------------------------------------------------------
class OS_API osWin32DebugInfoReader
{
public:
    osWin32DebugInfoReader(const osProcessHandle& hProcess);

    bool getModuleFromAddress(DWORD64 address, osFilePath& moduleFilePath, osInstructionPointer& moduleStartAddr) const;
    bool getFunctionFromAddress(DWORD64 address, DWORD64& functionStartAddress, gtString& functionName) const;
    bool getSourceCodeFromAddress(DWORD64 address, osFilePath& sourceCodeFile, int& lineNumber) const;
    bool fillStackFrame(osCallStackFrame& stackFrame);

private:
    // Do not allow to use my default constructor:
    osWin32DebugInfoReader();

private:
    // The queried process handle:
    osProcessHandle _hProcess;
};


#endif  // __osWIN32DEBUGINFOREADER
