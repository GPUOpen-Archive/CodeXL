//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerMidTierUtil.cpp
///
//==================================================================================


// Local.
#include <AMDTPowerProfilingMidTier/include/PowerProfilerMidTierUtil.h>

bool PowerProfilerMidTierUtil::isNonFailureResult(PPResult result)
{
    bool rc = false;
    if (PPR_NO_ERROR == result || PPR_NOT_SUPPORTED == result || PPR_DRIVER_ALREADY_IN_USE == result || PPR_DRIVER_VERSION_MISMATCH == result)
    {
        rc = true;
    }
    return rc;
}

const gtString& PowerProfilerMidTierUtil::CodeDescription(PPResult result)
{
    static gtString unknownValueDescription(L"Unknown value");
    static std::map<int, gtString> descriptions = {
        { PPR_NO_ERROR,                             gtString(L"PPR_NO_ERROR")},
        { PPR_WARNING_SMU_DISABLED,                 gtString(L"PPR_WARNING_SMU_DISABLED")},
        { PPR_WARNING_IGPU_DISABLED,                gtString(L"PPR_WARNING_IGPU_DISABLED")},
        { PPR_NOT_SUPPORTED,                        gtString(L"PPR_NOT_SUPPORTED")},
        { PPR_COMMUNICATION_FAILURE,                gtString(L"PPR_COMMUNICATION_FAILURE")},
        { PPR_POLLING_THREAD_ALREADY_RUNNING,       gtString(L"PPR_POLLING_THREAD_ALREADY_RUNNING")},
        { PPR_DB_CREATION_FAILURE,                  gtString(L"PPR_DB_CREATION_FAILURE")},
        { PPR_INVALID_SAMPLING_INTERVAL,            gtString(L"PPR_INVALID_SAMPLING_INTERVAL")},
        { PPR_DRIVER_ALREADY_IN_USE,                gtString(L"PPR_DRIVER_ALREADY_IN_USE")},
        { PPR_DRIVER_VERSION_MISMATCH,              gtString(L"PPR_DRIVER_VERSION_MISMATCH")},
        { PPR_REMOTE_ADDRESS_NOT_SET,               gtString(L"PPR_REMOTE_ADDRESS_NOT_SET")},
        { PPR_REMOTE_CONNECTION_ERROR,              gtString(L"PPR_REMOTE_CONNECTION_ERROR")},
        { PPR_REMOTE_HANDSHAKE_FAILURE,             gtString(L"PPR_REMOTE_HANDSHAKE_FAILURE")},
        { PPR_REMOTE_SESSION_CONFIGURATION_ERROR,   gtString(L"PPR_REMOTE_SESSION_CONFIGURATION_ERROR")},
        { PPR_REMOTE_APP_STOPPED,                   gtString(L"PPR_REMOTE_APP_STOPPED")},
        { PPR_HYPERVISOR_NOT_SUPPORTED,             gtString(L"PPR_HYPERVISOR_NOT_SUPPORTED")},
        { PPR_COUNTERS_NOT_ENABLED,                 gtString(L"PPR_COUNTERS_NOT_ENABLED")},
        { PPR_DB_MIGRATE_FAILURE,                   gtString(L"PPR_DB_MIGRATE_FAILURE")},
        { PPR_WRONG_PROJECT_SETTINGS,               gtString(L"PPR_WRONG_PROJECT_SETTINGS")},
        { PPR_UNKNOWN_FAILURE,                      gtString(L"PPR_UNKNOWN_FAILURE")}
    };

    auto it = descriptions.find(result);
    if (it != descriptions.end())
    {
        return it->second;
    }

    return unknownValueDescription;
}
