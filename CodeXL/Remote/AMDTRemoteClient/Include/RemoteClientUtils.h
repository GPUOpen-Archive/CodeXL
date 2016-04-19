//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RemoteClientUtils.h
///
//==================================================================================

#pragma once
#include <AMDTBaseTools/Include/gtString.h>

// This static class provides utilities related to remote GPU
// debugging and profiling from a client's perspective.
class RemoteClientUtils
{
public:
    static gtString GetPlatformMismatchMsg(const gtString& clientPlatform, const gtString& agentPlatform);
    static gtString GetVersionMismatchMsg(const gtString& clientVersion, const gtString& agentVersion);
};

