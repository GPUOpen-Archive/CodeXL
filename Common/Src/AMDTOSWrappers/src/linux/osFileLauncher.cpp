//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFileLauncher.cpp
///
//=====================================================================

//------------------------------ osFileLauncher.cpp ------------------------------

// Standard C:
#include <stdlib.h>

// POSIX:
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <common/osFileLauncherThread.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Desktop types are relevant only in Linux (and not in OS X):
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    #include <AMDTOSWrappers/Include/osDesktop.h>
#endif

// ---------------------------------------------------------------------------
// Name:        osLaunchFileInCurrentThread
// Description:
//  Launch the file using the machine default "open" application for that file.
//  The launch is done by the thread that calls this function.
// NOTE: The current implementation supports to scenarios: no command line arguments
//       or a single command line argument that is passed to the launched process.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/2/2007
// ---------------------------------------------------------------------------
bool osLaunchFileInCurrentThread(const gtString& fileToBeLaunched, const gtString& commandLineParameters)
{
    bool retVal = false;

    // Convert file path to UTF8
    std::string utf8FileToBeLaunched;
    fileToBeLaunched.asUtf8(utf8FileToBeLaunched);

    std::string utf8CommandLineParameters;
    commandLineParameters.asUtf8(utf8CommandLineParameters);

    // Fork this process into 2 processes (child and parent):
    pid_t childProcessId = ::fork();

    // If this is the child process:
    if (childProcessId == 0)
    {
        // GT_ASSERT are not allowed here will cause CodeXL to crash so for the child process all assertion handlers are removed
        // so the dialog will not open and the file logger will not be written
        gtUnregsiterAllAssertionFailureHandlers();

        pid_t childOfChildProcessId = ::fork();

        if (0 == childOfChildProcessId)
        {
            // Clear LD_LIBRARY_PATH in all cases since we are launching only system commands in this action:
            unsetenv("LD_LIBRARY_PATH");

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
            // Get the type of the desktop on which this application runs:
            osDesktopType desktopType = OS_GNOME_DESKTOP;
            bool rc1 = osGetDesktopType(desktopType);

            if (!rc1)
            {
                // Try xdg-open, which should work on both desktop types:
                if (commandLineParameters.isEmpty())
                {
                    const char* commandArgsXdg[] = { "xdg-open", utf8FileToBeLaunched.c_str(), 0 };
                    ::execvp("xdg-open", (char* const*)commandArgsXdg);
                }
                else
                {
                    const char* commandArgsXdg[] = { "xdg-open", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                    ::execvp("xdg-open", (char* const*)commandArgsXdg);
                }
            }
            else
            {
                // Try xdg-open, which should work on both desktop types:
                if (commandLineParameters.isEmpty())
                {
                    const char* commandArgsXdg[] = { "xdg-open", utf8FileToBeLaunched.c_str(), 0 };
                    ::execvp("xdg-open", (char* const*)commandArgsXdg);
                }
                else
                {
                    const char* commandArgsXdg[] = { "xdg-open", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                    ::execvp("xdg-open", (char* const*)commandArgsXdg);
                }

                // If that failed, try platform-specific options:
                if (desktopType == OS_GNOME_DESKTOP)
                {
                    // We are running on a GNOME desktop - use gnome-open to open the file:
                    // (The child process will be replaced with a process running gnome-open):
                    if (commandLineParameters.isEmpty())
                    {
                        const char* commandArgsGnome[] = { "gnome-open", utf8FileToBeLaunched.c_str(), 0 };
                        ::execvp("gnome-open", (char* const*)commandArgsGnome);
                    }
                    else
                    {
                        const char* commandArgsGnome[] = { "gnome-open", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                        ::execvp("gnome-open", (char* const*)commandArgsGnome);
                    }
                }
                else if (desktopType == OS_KDE_DESKTOP)
                {
                    // We are running on a KDE desktop - use gnome-open to open the file:
                    // (replace the child process with a process running tgnome-open):
                    if (commandLineParameters.isEmpty())
                    {
                        const char* commandArgsKDE[] = { "kfmclient", "openURL", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                        ::execvp("kfmclient", (char* const*)commandArgsKDE);
                    }
                    else
                    {
                        const char* commandArgsKDE[] = { "kfmclient", "openURL", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                        ::execvp("kfmclient", (char* const*)commandArgsKDE);
                    }
                }

                // If we got here - gnome-open / KFM client didn't run.
                // Try using mozilla:
                if (commandLineParameters.isEmpty())
                {
                    const char* commandArgsMozilla[] = { "mozilla", utf8FileToBeLaunched.c_str(), 0 };
                    ::execvp("mozilla", (char* const*)commandArgsMozilla);
                }
                else
                {
                    const char* commandArgsMozilla[] = { "mozilla", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), 0 };
                    ::execvp("mozilla", (char* const*)commandArgsMozilla);
                }

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

            // We are running on OS X, use the "open" command to open the file
            if (commandLineParameters.isEmpty())
            {
                const char* commandArgs[] = { "open", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), NULL };
                ::execvp("open", (char* const*)commandArgs);
            }
            else
            {
                const char* commandArgs[] = { "open", utf8FileToBeLaunched.c_str(), utf8CommandLineParameters.c_str(), NULL };
                ::execvp("open", (char* const*)commandArgs);
            }

#else
#error Unknown Linux variant!
#endif

                // If we reached here - an error occurred:
                // Exit the child process:
                _exit(-1);
            }
        }
        else
        {
            _exit(0);
        }
    }
    else
    {
        // We are in the parent process.

        // Wait for the child process to complete its action:
        int pidStatus;
        ::waitpid(childProcessId, &pidStatus, 0);
        GT_IF_WITH_ASSERT(pidStatus != -1)
        {
            retVal = true;
        }
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
        osFileLauncherThread* pFileLaunchingThread = new osFileLauncherThread(_fileToBeLaunched);
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
    (void)(fileToBeLaunched); // unused
    (void)(commandLineParameters); // unused
    (void)(retCode); // unused
    // TO_DO: LNX - implement me!
    return false;
}
