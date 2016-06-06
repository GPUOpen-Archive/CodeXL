//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file WinAppExp.h
///
//==================================================================================

#ifdef LIBRARY_EXPORTS
    #define LIBRARY_API __declspec(dllexport)
#else
    #define LIBRARY_API __declspec(dllimport)
#endif

extern "C" LIBRARY_API int  GetAppInfo(gtList<WindowsStoreAppInfo>& outAppInfoList);
