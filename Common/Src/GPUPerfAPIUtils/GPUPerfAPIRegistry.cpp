//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Functions for setting driver-specific registry keys or env variables
///         to enable perf counter collection
//==============================================================================

#include "GPUPerfAPIRegistry.h"

#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <atlbase.h>
    #include <atlstr.h>
#endif // _WIN32

#ifdef _WIN32
    static const LPCTSTR HSA_SOFTCP_ENV_VAR_NAME = _T("HSA_EMULATE_AQL");
    static const LPCTSTR HSA_SOFTCP_ENV_VAR_VALUE = _T("1");
#else
    static const char* HSA_SOFTCP_ENV_VAR_NAME = "HSA_EMULATE_AQL";
    static const char* HSA_SOFTCP_ENV_VAR_VALUE = "1";
#endif

bool SetHSASoftCPEnvVar(bool shouldPrintDbgMsg, std::string& strErrorMsg)
{
    bool retVal = true;
#ifdef _WIN32
    BOOL result = SetEnvironmentVariable(HSA_SOFTCP_ENV_VAR_NAME, HSA_SOFTCP_ENV_VAR_VALUE);
    retVal = (TRUE == result);
#else
    int result = setenv(HSA_SOFTCP_ENV_VAR_NAME, HSA_SOFTCP_ENV_VAR_VALUE, 1);
    retVal = (0 == result);
#endif

    if (!retVal)
    {
        strErrorMsg = "Error: Unable to enable HSA Performance Counters in Driver";
    }
    else
    {
        strErrorMsg = "Successfully enabled HSA Performance Counters in Driver";
    }

    if (shouldPrintDbgMsg)
    {
        std::cout << strErrorMsg << std::endl;
    }

    return retVal;
}

bool SetHSASoftCPEnvVar(bool shouldPrintDbgMsg)
{
    std::string strErrorMsg;
    return SetHSASoftCPEnvVar(shouldPrintDbgMsg, strErrorMsg);
}

bool SetHSASoftCPEnvVar(std::string& strErrorMsg)
{
    return SetHSASoftCPEnvVar(false, strErrorMsg);
}

bool UnsetHSASoftCPEnvVar(bool shouldPrintDbgMsg, std::string strErrorMsg)
{
    bool retVal = true;
#ifdef _WIN32
    BOOL result = SetEnvironmentVariable(HSA_SOFTCP_ENV_VAR_NAME, NULL);
    retVal = (TRUE == result);
#else
    int result = unsetenv(HSA_SOFTCP_ENV_VAR_NAME);
    retVal = (0 == result);
#endif

    if (!retVal)
    {
        strErrorMsg = "Error: Unable to disable HSA Performance Counters in Driver";
    }
    else
    {
        strErrorMsg = "Successfully disabled HSA Performance Counters in Driver";
    }

    if (shouldPrintDbgMsg)
    {
        std::cout << strErrorMsg << std::endl;
    }

    return retVal;
}

bool UnsetHSASoftCPEnvVar(bool shouldPrintDbgMsg)
{
    std::string strErrorMsg;
    return UnsetHSASoftCPEnvVar(shouldPrintDbgMsg, strErrorMsg);
}

bool UnsetHSASoftCPEnvVar(std::string& strErrorMsg)
{
    return UnsetHSASoftCPEnvVar(false, strErrorMsg);
}
