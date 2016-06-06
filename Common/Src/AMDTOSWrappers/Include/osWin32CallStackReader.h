//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32CallStackReader.h
///
//=====================================================================

//------------------------------ osWin32CallStackReader.h ------------------------------

#ifndef __OSWIN32CALLSTACKREADER
#define __OSWIN32CALLSTACKREADER

// Pre-deceleration:
class osCallStack;
class osCallStackFrame;
typedef struct _tagSTACKFRAME64 STACKFRAME64;

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osWin32DebugInfoReader.h>


// ----------------------------------------------------------------------------------
// Class Name:           PD_API osWin32CallStackReader
// General Description:
//   Reads the current call stack of a debugged process.
// Author:      AMD Developer Tools Team
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class OS_API osWin32CallStackReader
{
public:
    osWin32CallStackReader(osCallStack& callStack);
    osWin32CallStackReader(osProcessHandle hProcess, osThreadHandle hThread, osCallStack& callStack);
    osWin32CallStackReader(osProcessHandle hProcess, CONTEXT* pThreadExecutionContext, osCallStack& callStack);
    bool execute(bool hideSpyDLLsFunctions = true);
    void setMaxFrameCount(int frameCount) { m_maxFrames = frameCount; };

private:
    void setStackAddressSpaceType();
    bool fillStackFrame(const STACKFRAME64& winStackFrame, osCallStackFrame& stackFrame);

    // Disallow use of default constructor, copy constructor and assignment operator:
    osWin32CallStackReader() = delete;
    osWin32CallStackReader(const osWin32CallStackReader&) = delete;
    osWin32CallStackReader& operator=(const osWin32CallStackReader&) = delete;

private:
    // The queried thread owning process handle:
    osProcessHandle _hProcess;

    // The queried thread handle:
    osThreadHandle _hThread;

    // An execution context of a thread's who's call stack will be read:
    CONTEXT* _pThreadExecutionContext;

    // A reference to the output call stack:
    osCallStack& _callStack;

    // The debug information reader:
    osWin32DebugInfoReader _debugInfoReader;

    // Are we reading the current thread's calls stack:
    bool _isReadingCurrentThread;

    // Maximal number of frames allowed in the call stack:
    int m_maxFrames;
};


#endif  // __OSWIN32CALLSTACKREADER
