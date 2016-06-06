//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file WinPackages.h
///
//==================================================================================

class WinPackages
{
    WinPackages() = delete;
    WinPackages(const WinPackages&) = delete;
public:
    typedef gtList<WindowsStoreAppInfo>  AppInfoList;

    static int GetApplicationInfo(AppInfoList& outAppInfoList);
    static gtString UserIdApp(const gtString& FamilyName, const gtString& RelativeAppId);
};
