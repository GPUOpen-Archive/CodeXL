//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSingleApplicationInstance.cpp
///
//=====================================================================

//------------------------------ osSingleApplicationInstance.cpp ------------------------------

// POSIX:
// #include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osSingleApplicationInstance.h>


// ---------------------------------------------------------------------------
// Name:        osSingleApplicationInstance::osSingleApplicationInstance
// Description: Constructor
// Arguments:
//   applicationUniqueIdentifier - A unique string that identifies the application.
//                                 All application instances should use the same string.
//                                 This string should be unique for this application.
// Author:      AMD Developer Tools Team
// Date:        7/4/2008
// Implementation notes:
//   We create a file named /tmp/applicationUniqueIdentifier.dat and try to aquire a
//   lock for it. If we cannot aquire a lock, it means that anoter instance of the
//   application is running and holding the file locked.
// ---------------------------------------------------------------------------
osSingleApplicationInstance::osSingleApplicationInstance(const gtString& applicationUniqueIdentifier)
    : _isAnotherInstanceRunning(false), _fileDescriptor(-1)
{
    // Build the file path (/tmp/<applicationUniqueIdentifier>.dat):
    gtString filePath = L"/tmp/";
    filePath += applicationUniqueIdentifier;
    filePath += L".dat";

    // Try to open the file for writing, create the file if it does not exist:
    _fileDescriptor = ::open(filePath.asUTF8CharArray(), O_WRONLY | O_CREAT, 0666);

    if (_fileDescriptor == -1)
    {
        // We failed to open the file:
        _isAnotherInstanceRunning = true;
    }
    else
    {
        // A structure describing the file section that we would like to lock:
        // (Lock bytes [0, 1] for writing)
        struct flock fileSectionLockDescription;
        fileSectionLockDescription.l_whence = SEEK_SET;
        fileSectionLockDescription.l_start = 0;
        fileSectionLockDescription.l_len = 1;
        fileSectionLockDescription.l_type = F_WRLCK;

        // Try to lock the file [0,1] section for writing:
        int rc1 = fcntl(_fileDescriptor, F_SETLK, &fileSectionLockDescription);

        if (rc1 == -1)
        {
            // We failed to lock the file, this means that another instance
            // of the program is holding the file locked:
            _isAnotherInstanceRunning = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        osSingleApplicationInstance::~osSingleApplicationInstance
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        7/4/2008
// ---------------------------------------------------------------------------
osSingleApplicationInstance::~osSingleApplicationInstance()
{
    if (_fileDescriptor != -1)
    {
        // A structure describing the file section that we would like to unlock:
        // (Lock bytes [0, 1] for writing)
        struct flock fileSectionLockDescription;
        fileSectionLockDescription.l_whence = SEEK_SET;
        fileSectionLockDescription.l_start = 0;
        fileSectionLockDescription.l_len = 1;
        fileSectionLockDescription.l_type = F_UNLCK;

        // Ulock the file [0,1] section:
        int rc1 = fcntl(_fileDescriptor, F_SETLK, &fileSectionLockDescription);
        GT_ASSERT(rc1 != -1);

        // Close the file:
        ::close(_fileDescriptor);
        _fileDescriptor = -1;
    }
}


