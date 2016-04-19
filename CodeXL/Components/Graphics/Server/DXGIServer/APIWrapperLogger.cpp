//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Callback function provided to the AMDLoggingRegistration object in the APIWrapping code.
//=====================================================================================

#include "APIWrapperLogger.h"

void fprintf_callback(void* pUser, const AMDLogType type, char const* const msg)
{
    pUser;

    if (msg != NULL)
    {
        fprintf(stdout, "%d: %s\n", type, msg);
        fflush(stdout);
    }
}