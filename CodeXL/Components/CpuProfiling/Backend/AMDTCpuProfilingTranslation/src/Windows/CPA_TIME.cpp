//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPA_TIME.cpp
///
//==================================================================================

#include <AMDTCpuProfilingTranslation/inc/CPA_TIME.h>

// CPA_TIME struct contains the time since epoch
// Epoch is defined as the time 00:00:00 +0000 (UTC) on 1970-01-01.

// The Windows ticks are in 100 nanoseconds (10^-7).
#define WINDOWS_TICK_PER_SEC 10000000

// The windows epoch starts 1601-01-01 00:00:00.
// It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01 00:00:00).
#define SEC_TO_UNIX_EPOCH 11644473600LL


//TODO: [Suravee] : Verify this
HRESULT CPA_TIME_to_FILETIME(CPA_TIME cpaTime, FILETIME* pFileTime)
{
    if (NULL == pFileTime)
    {
        return E_INVALIDARG;
    }

    ULARGE_INTEGER time;

    // Convert CPA_TIME to 100-nanosec
    time.QuadPart = (cpaTime.second * WINDOWS_TICK_PER_SEC) + (cpaTime.microsec * 10);

    // Rebase to Windows EPOC
    time.QuadPart = time.QuadPart + (SEC_TO_UNIX_EPOCH * WINDOWS_TICK_PER_SEC);

    // Copy value into FILETIME
    pFileTime->dwHighDateTime = (DWORD)(time.HighPart);
    pFileTime->dwLowDateTime = (DWORD)(time.LowPart);

    return S_OK;
}

//TODO: [Suravee] : Verify this
HRESULT FILETIME_to_CPA_TIME(FILETIME fileTime, CPA_TIME* pCpaTime)
{
    if (NULL == pCpaTime)
    {
        return E_INVALIDARG;
    }

    ULARGE_INTEGER time;
    time.HighPart = fileTime.dwHighDateTime;
    time.LowPart = fileTime.dwLowDateTime;

    pCpaTime->second = static_cast<gtUInt32>((time.QuadPart / WINDOWS_TICK_PER_SEC) - SEC_TO_UNIX_EPOCH);
    //  pCpaTime->microsec = static_cast<gtUInt32>((time.QuadPart - (time.QuadPart / WINDOWS_TICK_PER_SEC)) / 10 );
    pCpaTime->microsec = static_cast<gtUInt32>((time.QuadPart - pCpaTime->second) / 10);

    return S_OK;
}
