//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePermissions.cpp
///
//=====================================================================

//------------------------------ osFilePermissions.cpp ------------------------------

// Windows API:
#include <Aclapi.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePermissions.h>


// General note:
// ============
// Linux permissions are divided to user, users group and others.
// On windows, a user can belong to any number of groups. Example: "Users", "Administrators", etc.
// This makes it hard to create a unified file permissions API. Instead, we supply a very basic API
// that matches both operating systems.
// Yaki - 15/6/2008


// The "Users" windows users group:
#define OS_WIN_EVERYONE_GROUP_NAME L"EVERYONE"


// ---------------------------------------------------------------------------
// Name:        osToWindowsStandardAccessTypesMask
// Description: Translates osFilePermissions mask to Windows standard access types mask.
// Author:      AMD Developer Tools Team
// Date:        15/6/2008
// ---------------------------------------------------------------------------
DWORD osToWindowsStandardAccessTypesMask(unsigned int filePermissionsMask)
{
    DWORD retVal = 0;

    if (filePermissionsMask & OS_READ_PERMISSION)
    {
        retVal |= FILE_GENERIC_READ;
    }

    if (filePermissionsMask & OS_WRITE_PERMISSION)
    {
        retVal |= FILE_GENERIC_WRITE;
    }

    if (filePermissionsMask & OS_EXECUTE_PERMISSION)
    {
        retVal |= FILE_GENERIC_EXECUTE;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osAddAllUsersFilePermissions
// Description: Adds permissions for all users for a given a file / directory.
// Arguments: filePath - The file path.
//            filePermissionsMask - A mask of osFilePermissions enumerators.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/6/2008
// ---------------------------------------------------------------------------
bool osAddAllUsersFilePermissions(const osFilePath& filePath, unsigned int filePermissionsMask)
{
    bool retVal = false;

    // Get the file's current DACL (file permissions) information:
    ACL* pCurrentFileDACL = NULL;
    PSECURITY_DESCRIPTOR pCurrentFileSecurityDescriptor = NULL;
    DWORD rc1 = GetNamedSecurityInfo(LPWSTR(filePath.asString().asCharArray()), SE_FILE_OBJECT,
                                     DACL_SECURITY_INFORMATION, NULL, NULL, &pCurrentFileDACL, NULL,
                                     &pCurrentFileSecurityDescriptor);

    GT_IF_WITH_ASSERT(rc1 == ERROR_SUCCESS)
    {
        // Translate the file permissions mask to Windows terminology:
        DWORD filePermissionsMaskWindows = osToWindowsStandardAccessTypesMask(filePermissionsMask);

        // Create a structure defining the new file's access control information:
        EXPLICIT_ACCESS fileNewAccessControlInfo;
        ZeroMemory(&fileNewAccessControlInfo, sizeof(EXPLICIT_ACCESS));
        fileNewAccessControlInfo.grfAccessPermissions = filePermissionsMaskWindows;
        fileNewAccessControlInfo.grfAccessMode = GRANT_ACCESS;
        fileNewAccessControlInfo.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
        fileNewAccessControlInfo.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        fileNewAccessControlInfo.Trustee.ptstrName = OS_WIN_EVERYONE_GROUP_NAME;

        // Create a new DACL (file permissions) information that contains the merge of the
        // current file's permissions and the added files permissions:
        ACL* pNewFileDACL = NULL;
        DWORD rc2 = SetEntriesInAcl(1, &fileNewAccessControlInfo, pCurrentFileDACL, &pNewFileDACL);

        GT_IF_WITH_ASSERT(rc2 == ERROR_SUCCESS)
        {
            // Set the file's permissions to be the new DACL information:
            DWORD rc3 = SetNamedSecurityInfo(LPWSTR(filePath.asString().asCharArray()), SE_FILE_OBJECT,
                                             DACL_SECURITY_INFORMATION, NULL, NULL, pNewFileDACL, NULL);

            GT_IF_WITH_ASSERT(rc3 == ERROR_SUCCESS)
            {
                retVal = true;
            }

            // Clean up:
            LocalFree((HLOCAL)pNewFileDACL);
        }

        // Clean up:
        LocalFree((HLOCAL)pCurrentFileSecurityDescriptor);
    }

    return retVal;
}

