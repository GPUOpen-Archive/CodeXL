//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBProcessWaiterThread.cpp
///
//==================================================================================

//------------------------------ pdGDBProcessWaiterThread.cpp ------------------------------

// POSIX:
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Infra:
#include <AMDTOSWrappers/Include/osSystemError.h>

// Local:
#include <src/pdGDBProcessWaiterThread.h>

// ---------------------------------------------------------------------------
// Name:        pdGDBProcessWaiterThread::pdGDBProcessWaiterThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        3/4/2012
// ---------------------------------------------------------------------------
pdGDBProcessWaiterThread::pdGDBProcessWaiterThread(osProcessId gdbProcessId)
    : osThread(L"pdGDBProcessWaiterThread"), _gdbProcessId(gdbProcessId), _continueWaiting(true)
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBProcessWaiterThread::~pdGDBProcessWaiterThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        3/4/2012
// ---------------------------------------------------------------------------
pdGDBProcessWaiterThread::~pdGDBProcessWaiterThread()
{

}

// ---------------------------------------------------------------------------
// Name:        pdGDBProcessWaiterThread::entryPoint
// Description:
// Author:      Uri Shomroni
// Date:        3/4/2012
// ---------------------------------------------------------------------------
int pdGDBProcessWaiterThread::entryPoint()
{
    while (_continueWaiting)
    {
        int processStatus = -1;
        int rpid = ::waitpid(_gdbProcessId, &processStatus, 0);

        if (_gdbProcessId == rpid)
        {
            // If the process terminated:
            if (WIFEXITED(processStatus) || WIFSIGNALED(processStatus))
            {
                // We can stop waiting:
                _continueWaiting = false;
            }

            // Otherwise, the process was only suspended or resumed. Continue waiting.
        }
        else // _gdbProcessId != rpid ==> -1 == rpid
        {
            // Get the system error number
            osSystemErrorCode lastSystemError = osGetLastSystemError();

            // EINTR means we were interrupted by a signal, possibly a SIGCHLD.
            // Continue waiting.
            if (EINTR != lastSystemError) // ==> ECHILD == lastSystemError
            {
                // We got some other error, most probably one saying the child process no longer exists.
                // Stop waiting:
                _continueWaiting = false;
            }
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Name:        pdGDBProcessWaiterThread::beforeTermination
// Description: The thread entry point
// Author:      Uri Shomroni
// Date:        3/4/2012
// ---------------------------------------------------------------------------
void pdGDBProcessWaiterThread::beforeTermination()
{
    _continueWaiting = false;
}
