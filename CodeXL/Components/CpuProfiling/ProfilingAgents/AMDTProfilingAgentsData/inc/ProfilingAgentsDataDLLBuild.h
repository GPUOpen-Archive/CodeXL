//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProfilingAgentsDataDLLBuild.h
///
//==================================================================================

//------------------------------ ProfilingAgentsDataDLLBuild.h ------------------------------

#ifndef _PROFILINGAGENTSDATADLLBUILD_H_
#define _PROFILINGAGENTSDATADLLBUILD_H_

// Under Win32 builds - define: AGENTDATA_API to be:
// - When building AMDTProfilingAgentsData.dll: __declspec(dllexport).
// - When building other projects:              __declspec(dllimport).

#if !defined(AMDT_PROFILINGAGENTSDATA_PRIVATE) && defined(_WIN32)
    #if defined(AMDT_PROFILINGAGENTSDATA_EXPORTS)
        #define AGENTDATA_API __declspec(dllexport)
    #else
        #define AGENTDATA_API __declspec(dllimport)
    #endif
#else
    #define AGENTDATA_API
#endif


#endif  // _PROFILINGAGENTSDATADLLBUILD_H_
