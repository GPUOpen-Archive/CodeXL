//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWin32DebuggerThread.cpp
///
//==================================================================================

//------------------------------ pdWin32DebuggerThread.cpp ------------------------------

// Local:
#include <src/pdWin32DebuggerThread.h>
#include <src/pdWin32ProcessDebugger.h>


// ---------------------------------------------------------------------------
// Name:        pdWin32DebuggerThread::pdWin32DebuggerThread
// Description: Constructor
// Arguments:   The process debugger that this thread will run.
// Author:      Yaki Tebeka
// Date:        28/3/2004
// ---------------------------------------------------------------------------
pdWin32DebuggerThread::pdWin32DebuggerThread(pdWin32ProcessDebugger& processDebugger)
    : osThread(L"pdWin32DebuggerThread"), _processDebugger(processDebugger)
{
}


// ---------------------------------------------------------------------------
// Name:        pdWin32DebuggerThread::~pdWin32DebuggerThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        28/3/2004
// ---------------------------------------------------------------------------
pdWin32DebuggerThread::~pdWin32DebuggerThread()
{
}


// ---------------------------------------------------------------------------
// Name:        pdWin32DebuggerThread::entryPoint
// Description:
//   The thread entry point.
//   This thread does the following:
//   a. Create the debugged process.
//   b. Waits for its debug events.
//   c. Terminates when the debugged process terminates (a termination
//      debug event arrives).
// Return Val:  int
// Author:      Yaki Tebeka
// Date:        28/3/2004
// ---------------------------------------------------------------------------
int pdWin32DebuggerThread::entryPoint()
{
    int retVal = 0;

    // Launch the debugged process:
    bool rc = _processDebugger.createDebuggedProcess();

    if (rc)
    {
        // TO_DO: When do we end the session - how do we know its over ?
        for (;;)
        {
            // Wait for and handle debug events:
            rc = _processDebugger.waitForDebugEvent();
        }
    }

    return retVal;
}



