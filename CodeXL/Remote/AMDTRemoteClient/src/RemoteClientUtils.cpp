//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RemoteClientUtils.cpp
///
//==================================================================================

#include "RemoteClientUtils.h"
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>
#include <sstream>
using namespace std;

gtString RemoteClientUtils::GetPlatformMismatchMsg(const gtString& clientPlatform, const gtString& agentPlatform)
{
    wstringstream msg;
    msg << DMN_STR_ERR_PLATFORM_MISMATCH_A << clientPlatform.asCharArray() << L" to " <<
        agentPlatform.asCharArray() << L" " << DMN_STR_ERR_PLATFORM_MISMATCH_B;
    return msg.str().c_str();
}

gtString RemoteClientUtils::GetVersionMismatchMsg(const gtString& clientVersion, const gtString& agentVersion)
{
    wstringstream msg;
    msg << DMN_STR_ERR_VERSION_MISMATCH_A << agentVersion.asCharArray() <<
        DMN_STR_ERR_VERSION_MISMATCH_B << clientVersion.asCharArray() << L".";
    return msg.str().c_str();
}
