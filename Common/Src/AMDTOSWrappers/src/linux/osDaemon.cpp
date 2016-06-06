//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDaemon.cpp
///
//=====================================================================

//------------------------------ osDaemon.cpp ------------------------------

// Standard C:
#include <stdio.h>
#include <stdlib.h>

// POSIX:
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osToAndFromString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDaemon.h>
#include <AMDTOSWrappers/Include/osChannel.h>


// ---------------------------------------------------------------------------
// Name:        osForkAndSwitchExecutionToChildProcess
// Description: Calls fork(), exits the parent process and continues execution
//              at the child process.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/4/2008
// ---------------------------------------------------------------------------
bool osForkAndSwitchExecutionToChildProcess()
{
    bool retVal = true;

    // Fork into 2 processes:
    int status = fork();

    switch (status)
    {
        case -1:
        {
            // An error occurred:
            retVal = false;
            GT_ASSERT(false);
        }

        case 0:
        {
            // We are inside the child process - continue running.
        }
        break;

        default:
        {
            // We are inside the parent process - exit the process:
            exit(0);
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCloseAllOpenFileDescriptors
// Description: Closes all this process open file descriptors.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/4/2008
// ---------------------------------------------------------------------------
bool osCloseAllOpenFileDescriptors()
{
    bool retVal = false;

    // Get the maximal file descriptor number that can be opened by this process:
    struct rlimit resourceLimit;
    resourceLimit.rlim_max = 0;
    int rc1 = getrlimit(RLIMIT_NOFILE, &resourceLimit);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(0 < resourceLimit.rlim_max)
        {
            // Close all open file descriptors:
            for (unsigned int i = 0; i < resourceLimit.rlim_max; i++)
            {
                (void)close(i);
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osMapStandardIOStreamsToNULL
// Description:
//   Opens the the three standard I/O descriptors (stdin, stdout, stderr) and
//   connect them to a harmless I/O device (/dev/null).
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/4/2008
// ---------------------------------------------------------------------------
bool osMapStandardIOStreamsToNULL()
{
    bool retVal = false;

    // Open the /dev/null special file. This will redirect stdin into it:
    int devNULLFileDescriptor = open("/dev/null", O_RDWR);
    GT_IF_WITH_ASSERT(devNULLFileDescriptor != -1)
    {
        // Redirect stdout into /dev/null:
        int rc1 = dup(devNULLFileDescriptor);

        // Redirect stderr into /dev/null:
        int rc2 = dup(devNULLFileDescriptor);

        GT_IF_WITH_ASSERT((rc1 != -1) && (rc2 != -1))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWriteProcessIdToVarRun
// Description: Writes this process id to /var/run/<process name>.pid
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/4/2008
// ---------------------------------------------------------------------------
bool osWriteProcessIdToVarRun()
{
    bool retVal = false;

    // Get the current application name:
    gtString applicationName;
    bool rc1 = osGetCurrentApplicationName(applicationName);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Calculate the /var/run/<process name>.pid path:
        gtString varFilePathAsString = L"/var/run/";
        varFilePathAsString += applicationName;
        varFilePathAsString += L".pid";

        // Open the /var/run/<process name>.pid file:
        osFilePath varFilePath(varFilePathAsString);
        osFile varRunFile;
        bool rc2 = varRunFile.open(varFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Get this process id:
            osProcessId thisProcessId = osGetCurrentProcessId();

            // Write it into the /var/run/<process name>.pid file:
            gtString thisProcessIdAsString;
            bool rc3 = osProcessIdToString(thisProcessId, thisProcessIdAsString);
            GT_IF_WITH_ASSERT(rc3)
            {
                varRunFile << thisProcessIdAsString;
                retVal = true;
            }

            // Close the file:
            varRunFile.close();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osMakeThisProcessDaemon
// Description: Performs actions required to make this process a "well behaved"
//              daemon (background server).
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/4/2008
// Implementation notes:
//   To become a "well behaved" daemon, we perform the following actions:
//   a. Put this process in the background (done using fork).
//   b. Close all open file descriptors inherited from our parent process.
//   c. Create a new session and thus, a new process group with our process
//      as the new session leader.
//   d. Fork again to become the second child of the of the process group.
//      This prevents the process from re-acquiring a controlling terminal.
//   e. chdir() to / to enable unmounting the file system from which the daemon
//      was invoked.
//   f. Set umask to 0 to prevent problems with file creation using a mask inherited
//      from my parent process.
//   g. Open the the three standard I/O descriptors and connect them to a harmless
//      I/O device (/dev/null).
//   h. Write this daemon process id to /var/run/<process name>.pid
//
// For more details - see Linux Journal, Linux Network Programming, Part 2
// (http://www.linuxjournal.com/article/2335)
// ---------------------------------------------------------------------------
bool osMakeThisProcessDaemon()
{
    bool retVal = false;

    // a. Put this process in the background (done using fork).
    bool rc1 = osForkAndSwitchExecutionToChildProcess();
    GT_IF_WITH_ASSERT(rc1)
    {
        // b. Close all open file descriptors inherited from our parent process.
        bool rc2 = osCloseAllOpenFileDescriptors();
        GT_IF_WITH_ASSERT(rc2)
        {
            // c. Create a new session and thus, a new process group with our process as the new session leader:
            int rc3 = setsid();
            GT_IF_WITH_ASSERT(rc3 != -1)
            {
                // d. Fork again to become the second child of the of the new process group.
                //    (This prevents this process from re-acquiring a controlling terminal).
                bool rc4 = osForkAndSwitchExecutionToChildProcess();
                GT_IF_WITH_ASSERT(rc4)
                {
                    // e. chdir() to / to enable un-mounting the file system from which the daemon was invoked:
                    int rc5 = chdir("/");
                    GT_IF_WITH_ASSERT(rc5 == 0)
                    {
                        // f. Set umask to 0 to prevent problems with file creation using a mask inherited
                        // from my parent process:
                        umask(0);

                        // g. Open the the three standard I/O descriptors and connect them to a harmless
                        //    I/O device (/dev/null).
                        bool rc6 = osMapStandardIOStreamsToNULL();
                        GT_IF_WITH_ASSERT(rc6)
                        {
                            // h. Write this daemon process id to /var/run/<process name>.pid
                            bool rc7 = osWriteProcessIdToVarRun();
                            (void)(rc7);

                            // We don't require rc7 to be true, since we don't always have permission to write in /var/run/
                            retVal = true;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

