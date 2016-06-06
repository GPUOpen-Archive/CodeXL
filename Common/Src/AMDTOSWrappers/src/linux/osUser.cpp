//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osUser.cpp
///
//=====================================================================

//------------------------------ osUser.cpp ------------------------------

// POSIX:
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osUser.h>


// ---------------------------------------------------------------------------
// Name:        osGetCurrentUserName
// Description: Retrieves the name of user that is associated with the current
//              process.
// Arguments: currUserName - Will get the current user's name.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/6/2007
// ---------------------------------------------------------------------------
bool osGetCurrentUserName(gtString& currUserName)
{
    bool retVal = false;

    // Get the real user ID of the current process:
    uid_t currUserId = ::getuid();

    // Get this user password database record:
    struct passwd* pUserPassRecord = ::getpwuid(currUserId);
    GT_IF_WITH_ASSERT(pUserPassRecord != NULL)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pUserPassRecord->pw_name != NULL)
        {
            // Output the user name:
            currUserName.fromASCIIString(pUserPassRecord->pw_name);
            retVal = true;
        }
    }

    return retVal;
}

bool osGetProcessUserName(osProcessId processId, gtString& userName)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "/proc/%d/status", processId);

    bool ret = false;
    int fd = open(buffer, O_RDONLY, 0);

    if (-1 != fd)
    {
        int len = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (0 < len)
        {
            uid_t userId = 0;

            buffer[len] = '\0';
            char* pLine = buffer;

            for (char* pNewLine = NULL; NULL != pLine; pLine = pNewLine)
            {
                pNewLine = strchr(pLine, '\n');

                if (NULL != pNewLine)
                {
                    *pNewLine++ = '\0';
                }

                if (0 == memcmp(pLine, "Uid:", 4))
                {
                    pLine += 4;

                    while (isspace(*pLine))
                    {
                        pLine++;
                    }

                    // Skip the "Real" UID
                    while (!isspace(*pLine))
                    {
                        pLine++;
                    }

                    while (isspace(*pLine))
                    {
                        pLine++;
                    }

                    userId = strtol(pLine, &pLine, 10);

                    struct passwd* pw = getpwuid(userId);

                    if (NULL != pw && NULL != pw->pw_name)
                    {
                        userName.fromASCIIString(pw->pw_name);
                    }
                    else
                    {
                        userName.makeEmpty();
                    }

                    ret = true;
                    break;
                }
            }
        }
    }

    return ret;
}
