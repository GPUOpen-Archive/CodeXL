//==================================================================================
// Copyright (c) 2017, Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CXLTaskInfoDLLBuild.h
///
//==================================================================================

#pragma once

// Under Win32 builds - define: TASKINFO_API to be:
// - When building CXLTaskInfo.dll: __declspec(dllexport).
// - When building other projects:  __declspec(dllimport).

#if defined(_WIN32)
    #if defined(CXL_TASK_INFO_EXPORTS)
        #define TASKINFO_API __declspec(dllexport)
    #else
        #define TASKINFO_API __declspec(dllimport)
    #endif
#else
    #define TASKINFO_API
#endif
