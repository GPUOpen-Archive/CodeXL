//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileLauncher.cpp
///
//=====================================================================

//------------------------------ osFileLauncher.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <ShellAPI.h>

// Local:
#include <common/osFileLauncherThread.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>


// ---------------------------------------------------------------------------
// Name:        osLaunchFileInCurrentThread
// Description:
//  Launch the file using the machine default "open" application for that file.
//  The launch is done by the thread that calls this function.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/2/2007
// ---------------------------------------------------------------------------
bool osLaunchFileInCurrentThread(const gtString& fileToBeLaunched, const gtString& commandLineParameters)
{
    bool retVal = false;

    // Launch the file in the current thread:
    HINSTANCE result = nullptr;

    if (commandLineParameters.isEmpty())
    {
        result = ShellExecute(NULL, L"open", fileToBeLaunched.asCharArray(),
                              NULL, NULL, SW_SHOW);
    }
    else
    {
        gtString csCommandLine = commandLineParameters;
        csCommandLine.append(fileToBeLaunched);

        result = ShellExecute(NULL, gtString(L"Open").asCharArray(), gtString(L"Explorer.exe").asCharArray(), csCommandLine.asCharArray(), NULL, SW_SHOWNORMAL);
    }

    if ((size_t)result > HINSTANCE_ERROR)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFileLauncher::launchFile
// Description: Launch the file using the machine default "open" application
//              for that file.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2004
// ---------------------------------------------------------------------------
bool osFileLauncher::launchFile()
{
    bool retVal = false;

    if (_launchFileInDifferentThread)
    {
        // Launch the requested file in a different thread. This prevents the
        // calling thread from waiting until the file "open" application finish
        // loading:
        // Notice: osFileLauncherThread deletes its instance when it finish its task.
        osFileLauncherThread* pFileLaunchingThread = new osFileLauncherThread(_fileToBeLaunched, _commandLineParameters);
        retVal = pFileLaunchingThread->execute();
    }
    else
    {
        // Launch the requested file in the current thread:
        retVal = osLaunchFileInCurrentThread(_fileToBeLaunched, _commandLineParameters);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLaunchFileAndGetReturnCode
// Description: launches fileToBeLaunched with arguments commandLineParameters
//              and outputs the return code to retCode
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/3/2009
// ---------------------------------------------------------------------------
bool osLaunchFileAndGetReturnCode(const gtString& fileToBeLaunched, const gtString& commandLineParameters, long& retCode)
{
    bool retVal = false;

    SHELLEXECUTEINFO shellExecInfo = {0};
    shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shellExecInfo.hwnd = NULL;                                              // Don't create in an existing window
    shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;    // Don't close the process handle when it exits, and don't show visible errors to the user
    gtString verb = L"open";
    shellExecInfo.lpVerb = verb.asCharArray();
    shellExecInfo.lpFile = fileToBeLaunched.asCharArray();

    if (!commandLineParameters.isEmpty())
    {
        shellExecInfo.lpParameters = commandLineParameters.asCharArray();
    }
    else
    {
        shellExecInfo.lpParameters = NULL;
    }

    shellExecInfo.nShow = SW_SHOWNA;                                        // Don't make the launched application window active
    shellExecInfo.hProcess = NULL;                                          // Resetting the process handle member.

    BOOL rcShellExecute = ::ShellExecuteEx(&shellExecInfo);

    if (rcShellExecute == TRUE)
    {
        // Make sure that the process ran correctly:
        HINSTANCE shellExecResult = shellExecInfo.hInstApp;

        if ((size_t)shellExecResult > HINSTANCE_ERROR)
        {
            // Get the process handle:
            osProcessHandle hProc = shellExecInfo.hProcess;

            if (hProc != NULL)
            {
                // Make sure the process exits before going on:
                ::WaitForSingleObject(hProc, INFINITE);

                // Get the return code:
                DWORD retCodeAsDWORD = 0;
                BOOL rcExitCode = ::GetExitCodeProcess(hProc, &retCodeAsDWORD);

                if (rcExitCode == TRUE)
                {
                    retCode = (long)retCodeAsDWORD;
                    retVal = true;
                }
            }
        }
    }

    // If we got a process handle at any point, close it:
    if (shellExecInfo.hProcess != NULL)
    {
        ::CloseHandle(shellExecInfo.hProcess);
    }

    return retVal;
}
