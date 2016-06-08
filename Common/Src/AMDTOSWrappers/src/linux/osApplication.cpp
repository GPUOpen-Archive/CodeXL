//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osApplication.cpp
///
//=====================================================================

//------------------------------ osApplication.cpp ------------------------------

// POSIX:
#include <limits.h>
#include <unistd.h>

// Infra:
#include <VersionInfo/VersionInfo.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>


// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationPath
// Description: Returns the current application path (exe full path).
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/1/2007
// Implementation notes:
//  On Linux, the directory "/proc" is a virtual filesystem that contains
//  information about running processes.
//  To get the current application path, we read the content of
//  /proc/<this process id>/exe, which is a symbolic link to the
//  executable that created this process.
//
//  Notice that we can use libproc, a library designed for reading
//  the "/proc" virtual filesystem, but I think its an overkill
//  for what we do. (run ldd /bin/ps to see where it resides).
//
// For more information about the "/proc" irtual filesystem, see:
// - man proc
// - http://www.linux.com/guides/Linux-Filesystem-Hierarchy/proc.shtml
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationPath(osFilePath& applicationPath, bool convertToLower)
{
    (void)(convertToLower); // unused
    bool retVal = false;

    // Get this process id:
    pid_t thisProcessId = ::getpid();

    // "/proc/<this process id>/exe" is a symbolic link to the
    // executable that created this process.
    gtASCIIString symLinkPath = "/proc/";
    symLinkPath.appendFormattedString("%d/exe", thisProcessId);

    // Read the symbolic link content:
    char buff[PATH_MAX + 1];
    int rc1 = ::readlink(symLinkPath.asCharArray(), buff, PATH_MAX);
    GT_IF_WITH_ASSERT(rc1 != -1)
    {
        // NULL terminate the path we got:
        buff[rc1] = 0;

        // Output it:
        gtString bufferAsString;
        bufferAsString.fromUtf8String(buff);
        applicationPath.setFullPathFromString(bufferAsString);

        retVal = true;
    }

    return retVal;
}

bool osSupportWindowsStoreApps()
{
    return false;
}

bool osLaunchSuspendedWindowsStoreApp(const gtString& userModelId,
                                      const gtString& arguments,
                                      osProcessId& processId,
                                      osProcessHandle& processHandle,
                                      osFilePath& executablePath)
{
    (void)(userModelId); // unused
    (void)(arguments); // unused
    (void)(processId); // unused
    (void)(processHandle); // unused
    (void)(executablePath); // unused
    return false;
}

bool osResumeSuspendedWindowsStoreApp(const osProcessHandle& processHandle, bool closeHandle)
{
    (void)(processHandle); // unused
    (void)(closeHandle); // unused
    return false;
}

bool osEnumerateInstalledWindowsStoreApps(gtList<WindowsStoreAppInfo>& storeApps)
{
    (void)(storeApps); // unused
    return false;
}

bool osDetermineIsWindowsStoreApp64Bit(const gtString& userModelId)
{
    (void)(userModelId); // unused
    return false;
}

bool osGetWindowsStoreAppExecutable(const gtString& userModelId, gtString& exeFullPath)
{
    (void)(userModelId); // unused
    (void)(exeFullPath); // unused
    return false;
}
// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationVersion
// Description: Returns the current application version.
// Author:      AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
OS_API void osGetApplicationVersion(osProductVersion& applicationVersion)
{
    // Call the common implementation.
    osGetApplicationVersionFromMacros(applicationVersion);
}

// ---------------------------------------------------------------------------
// Name:        GetExecutedApplicationType
// Description: Returns the type of application we're running inside -
//              standalone or Visual Studio plug-in
// Author:      AMD Developer Tools Team
// Date:        Jul-26, 2015
// ---------------------------------------------------------------------------
OS_API osExecutedApplicationType GetExecutedApplicationType()
{
    return OS_STANDALONE_APPLICATION_TYPE;
}

// ---------------------------------------------------------------------------
// Name:        SetExecutedApplicationType
// Description: By default the app type is standalone, so only the VS plug-in should call
//              this function to let interested code know we're running inside Visual Studio
// Author:      AMD Developer Tools Team
// Date:        Jul-26, 2015
// ---------------------------------------------------------------------------
OS_API void SetExecutedApplicationType(osExecutedApplicationType appType)
{
    (void)(appType); // unused
    // This function is a no-op on Linux
}