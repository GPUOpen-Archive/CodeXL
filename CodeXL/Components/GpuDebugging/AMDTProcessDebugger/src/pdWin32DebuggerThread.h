//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32DebuggerThread.h
///
//==================================================================================

//------------------------------ pdWin32DebuggerThread.h ------------------------------

#ifndef __PDWIN32DEBUGGERTHREAD
#define __PDWIN32DEBUGGERTHREAD

// Pre-declaration:
class pdWin32ProcessDebugger;

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>

// ----------------------------------------------------------------------------------
// Class Name:           pdWin32DebuggerThread : public osThread
// General Description:
//   A thread that creates the debugged process and listens to its
//   debug events.
// Author:               Yaki Tebeka
// Creation Date:        28/3/2004
// ----------------------------------------------------------------------------------
class pdWin32DebuggerThread : public osThread
{
public:
    pdWin32DebuggerThread(pdWin32ProcessDebugger& processDebugger);
    virtual ~pdWin32DebuggerThread();

protected:
    // Overrides osThread:
    virtual int entryPoint();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    pdWin32DebuggerThread() = delete;
    pdWin32DebuggerThread(const pdWin32DebuggerThread&) = delete;
    pdWin32DebuggerThread& operator=(const pdWin32DebuggerThread&) = delete;

private:
    // The process debugger that this thread will run:
    pdWin32ProcessDebugger& _processDebugger;
};

#endif  // __PDWIN32DEBUGGERTHREAD
