//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file siAMDTSystemInformationHelper.cpp
///
//==================================================================================

/// AMDTSystemInformationHelper.cpp : Defines the entry point for the console application.
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#include <stdio.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <tchar.h>
#endif

#include "../inc/siOpenCLInformationCollector.h"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    int _tmain(int argc, _TCHAR* argv[])
#else
    int main(int argc, char* argv[])
#endif
{
    int exitCode = 0;

    gtString pipeName;

    if (argc > 1)
    {
        // getting pipe name
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        pipeName = gtString(argv[1]);
#else
        pipeName.fromASCIIString(argv[1]);
#endif
    }

    if (!pipeName.isEmpty())
    {
        siOpenCLInformationCollector sysInfo;
        bool rcGetInfo = sysInfo.GenerateAndSendOpenCLDevicesInformation(pipeName);
        exitCode = rcGetInfo ? 0 : 1;
    }

    return exitCode;
}
