//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osUser.cpp
///
//=====================================================================

//------------------------------ osUser.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osUser.h>

// The maximal user name size:
#define OS_MAX_USER_NAME_SIZE 256


// ---------------------------------------------------------------------------
// Name:        osGetCurrentUserName
// Description: Retrieves the name of user that is associated with the current
//              thread.
// Arguments: currUserName - Will get the current user's name.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/6/2007
// ---------------------------------------------------------------------------
bool osGetCurrentUserName(gtString& currUserName)
{
    bool retVal = false;

    // Get the current user name:
    wchar_t buff[OS_MAX_USER_NAME_SIZE];
    DWORD buffSize = OS_MAX_USER_NAME_SIZE;
    BOOL rc1 = ::GetUserName(buff, &buffSize);
    GT_IF_WITH_ASSERT(rc1 != FALSE)
    {
        // Output the user name:
        currUserName = buff;

        retVal = true;
    }

    return retVal;
}

bool osIsUserAdministrator()
{
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid;
    BOOL ret = AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &sid);

    if (ret)
    {
        ret = FALSE;
        CheckTokenMembership(NULL, sid, &ret);
        FreeSid(sid);
    }

    return FALSE != ret;
}

bool osGetProcessUserName(osProcessId processId, gtString& userName)
{
    bool ret = false;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);

    if (NULL != hProcess)
    {
        HANDLE hToken;

        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
        {
            ret = true;

            DWORD lenNeeded = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &lenNeeded);

            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                HANDLE hHeap = GetProcessHeap();
                TOKEN_USER* pUserData = static_cast<TOKEN_USER*>(HeapAlloc(hHeap, 0, lenNeeded));

                if (NULL != pUserData)
                {
                    if (GetTokenInformation(hToken, TokenUser, pUserData, lenNeeded, &lenNeeded))
                    {
                        wchar_t accountName[OS_MAX_USER_NAME_SIZE];
                        wchar_t domainName[OS_MAX_USER_NAME_SIZE];

                        DWORD usize = OS_MAX_USER_NAME_SIZE;
                        DWORD dsize = OS_MAX_USER_NAME_SIZE;

                        SID_NAME_USE sidType = SidTypeUnknown;

                        if (LookupAccountSidW(NULL, pUserData->User.Sid, accountName, &usize, domainName, &dsize, &sidType))
                        {
                            if (0 < dsize)
                            {
                                userName.assign(domainName, dsize);
                                userName.append(L'\\');
                            }
                            else
                            {
                                userName.makeEmpty();
                            }

                            if (0 < usize)
                            {
                                userName.append(accountName, usize);
                            }
                        }
                    }

                    HeapFree(hHeap, 0, pUserData);
                }
            }

            CloseHandle(hToken);
        }

        CloseHandle(hProcess);
    }

    return ret;
}

bool osSetPrivilege(void*& tokenHandle, wchar_t* pPrivilege, bool enablePrivilege, bool closeHandle)
{
    bool ret = false;

    HANDLE hToken = tokenHandle;

    if (NULL != hToken || OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        TOKEN_PRIVILEGES tp = { 0 };

        LUID luid;
        DWORD cb = sizeof(TOKEN_PRIVILEGES);

        if (!LookupPrivilegeValueW(NULL, pPrivilege, &luid))
        {
            return FALSE;
        }

        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = enablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

        if (AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL))
        {
            tokenHandle = hToken;
            ret = true;
        }
        else if (NULL == tokenHandle)
        {
            CloseHandle(hToken);
        }
    }

    if (closeHandle && NULL != tokenHandle)
    {
        CloseHandle(tokenHandle);
        tokenHandle = NULL;
    }

    return ret;
}
